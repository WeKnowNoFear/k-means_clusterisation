#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/kmeans.h"
#include "../include/utils.h"

#define EPSILON 1e-6

/*
 * Инициализация центроидов методом k-means++.
 *
 * Идея алгоритма:
 * 1. Первый центроид выбирается случайно
 * 2. Каждый следующий выбирается с вероятностью,
 *    пропорциональной квадрату расстояния до ближайшего центроида
 *
 * Это позволяет:
 * - улучшить сходимость
 * - избежать плохой инициализации
 *
 * Параллелизм:
 * - вычисление расстояний распараллелено (OpenMP)
 *
 * data       — массив точек
 * n_points   — количество точек
 * k          — число центроидов
 * dim        — размерность пространства
 * centroids  — массив центроидов (выход)
 */
void kmeans_init_plus_plus(const Point *data, int n_points, int k, int dim, Point *centroids)
{
    if (!data || !centroids || n_points <= 0 || k <= 0 || k > n_points)
        return;

    // Массив минимальных квадратов расстояний до ближайшего центроида
    double *min_dist_sq = malloc(sizeof(double) * n_points);
    if (!min_dist_sq)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    // Выбор первого центроида случайным образом
    int first = rand() % n_points;

    // Копирование координат
    memcpy(centroids[0].coords, data[first].coords, sizeof(double) * dim);

    // Вычисление расстояний до первого центроида (параллельно)
#pragma omp parallel for
    for (int i = 0; i < n_points; i++)
        min_dist_sq[i] = distance_sq(&data[i], &centroids[0], dim);

    // Выбор остальных центроидов
    for (int c = 1; c < k; c++)
    {
        double sum = 0.0;

        // Сумма вероятностей
        for (int i = 0; i < n_points; i++)
            sum += min_dist_sq[i];

        int next_idx;

        if (sum <= 0.0)
        {
            // Если все расстояния нулевые — выбираем случайную точку
            next_idx = rand() % n_points;
        }
        else
        {
            // Выбор точки с вероятностью ~ расстоянию
            double r = ((double)rand() / RAND_MAX) * sum;
            double acc = 0.0;

            next_idx = n_points - 1;

            for (int i = 0; i < n_points; i++)
            {
                acc += min_dist_sq[i];
                if (acc >= r)
                {
                    next_idx = i;
                    break;
                }
            }
        }

        // Копирование выбранной точки в центроид
        memcpy(centroids[c].coords, data[next_idx].coords, sizeof(double) * dim);

        // Обновление минимальных расстояний (параллельно)
#pragma omp parallel for
        for (int i = 0; i < n_points; i++)
        {
            double d = distance_sq(&data[i], &centroids[c], dim);
            if (d < min_dist_sq[i])
                min_dist_sq[i] = d;
        }
    }

    free(min_dist_sq);
}

/*
 * Основной алгоритм K-means.
 *
 * Этапы:
 * 1. Назначение каждой точки ближайшему центроиду
 * 2. Пересчёт центроидов как среднего значений точек
 * 3. Проверка сходимости
 *
 * Условие остановки:
 * - нет изменений кластеров
 * - или достигнут max_iter
 *
 * Параллелизм:
 * - распараллелен шаг назначения кластеров
 * - используется локальная агрегация для центроидов
 * - защита через critical и atomic
 *
 * data       — точки
 * n_points   — количество точек
 * k          — число кластеров
 * dim        — размерность
 * labels     — метки кластеров (выход)
 * centroids  — центроиды (выход)
 * max_iter   — максимальное число итераций
 */
void kmeans(const Point *data, int n_points, int k, int dim, int *labels, Point *centroids, int max_iter)
{
    if (!data || !labels || !centroids || n_points <= 0 || k <= 0 || max_iter <= 0 || k > n_points)
        return;

    /* Инициализация центроидов */
    for (int i = 0; i < k; i++)
    {
        if (!centroids[i].coords)
        {
            centroids[i].coords = malloc(sizeof(double) * dim);
            if (!centroids[i].coords)
            {
                fprintf(stderr, "Centroid allocation failed\n");
                return;
            }
        }
    }

    // Инициализация меток
    for (int i = 0; i < n_points; i++)
        labels[i] = -1;

    // k-means++ инициализация
    kmeans_init_plus_plus(data, n_points, k, dim, centroids);

    /* Основной цикл алгоритма */
    for (int iter = 0; iter < max_iter; iter++)
    {
        int changed = 0;

        /* Шаг 1: назначение точек кластерам */
#pragma omp parallel for
        for (int i = 0; i < n_points; i++)
        {
            double min_dist = distance_sq(&data[i], &centroids[0], dim);
            int cluster = 0;

            // Поиск ближайшего центроида
            for (int j = 1; j < k; j++)
            {
                double d = distance_sq(&data[i], &centroids[j], dim);
                if (d < min_dist)
                {
                    min_dist = d;
                    cluster = j;
                }
            }

            // Проверка изменения кластера
            if (labels[i] != cluster)
            {
                labels[i] = cluster;

                // Атомарная запись (без race condition)
#pragma omp atomic write
                changed = 1;
            }
        }

        /* Если ничего не изменилось — сходимость достигнута */
        if (!changed)
            break;

        /* Шаг 2: пересчёт центроидов */

        // Суммы координат для каждого кластера
        double *sum = calloc(k * dim, sizeof(double));

        // Количество точек в каждом кластере
        int *count = calloc(k, sizeof(int));

        if (!sum || !count)
        {
            fprintf(stderr, "Memory allocation failed\n");
            free(sum);
            free(count);
            return;
        }

        /* Параллельный сбор данных */
#pragma omp parallel
        {
            double *local_sum = calloc(k * dim, sizeof(double));
            int *local_count = calloc(k, sizeof(int));

#pragma omp for
            for (int i = 0; i < n_points; i++)
            {
                int c = labels[i];

                if (c >= 0 && c < k)
                {
                    // Суммирование координат
                    for (int d = 0; d < dim; d++)
                        local_sum[c * dim + d] += data[i].coords[d];

                    local_count[c]++;
                }
            }

            // Сведение результатов потоков
#pragma omp critical
            {
                for (int i = 0; i < k * dim; i++)
                    sum[i] += local_sum[i];

                for (int i = 0; i < k; i++)
                    count[i] += local_count[i];
            }

            free(local_sum);
            free(local_count);
        }

        /* Обновление центроидов */
        for (int i = 0; i < k; i++)
        {
            if (count[i] > 0)
            {
                for (int d = 0; d < dim; d++)
                {
                    double new_val = sum[i * dim + d] / count[i];

                    // Проверка изменения центроида
                    if (fabs(centroids[i].coords[d] - new_val) > EPSILON)
                        changed = 1;

                    centroids[i].coords[d] = new_val;
                }
            }
            else
            {
                // Если кластер пуст — выбираем случайную точку
                memcpy(centroids[i].coords,
                       data[rand() % n_points].coords,
                       sizeof(double) * dim);
            }
        }

        free(sum);
        free(count);

        // Проверка сходимости
        if (!changed)
            break;
    }
}