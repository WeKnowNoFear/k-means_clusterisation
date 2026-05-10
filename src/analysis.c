#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "../include/analysis.h"
#include "../include/kmeans.h"
#include "../include/utils.h"

/*
 * Поиск оптимального числа кластеров k
 * с использованием силуэтного коэффициента.
 *
 * Идея:
 * - перебираем значения k от 2 до max_k
 * - для каждого k запускаем k-means
 * - оцениваем качество с помощью silhouette score
 * - выбираем k с максимальным значением метрики
 *
 * Параллелизм:
 * - внешний цикл по k распараллелен (OpenMP)
 * - каждый поток работает со своим набором k
 * - используется локальное накопление результата
 * - затем критическая секция для выбора глобального лучшего k
 *
 * data     — массив точек
 * n_points — количество точек
 * max_k    — максимальное значение k
 * dim      — размерность пространства
 *
 * return:
 *   оптимальное значение k
 */
int find_best_k(const Point *data, int n_points, int max_k, int dim)
{
    double best_score = -1.0;
    int best_k = 2;

#pragma omp parallel
    {
        // Локальные переменные для каждого потока
        double local_best_score = -1.0;
        int local_best_k = 2;

#pragma omp for
        for (int k = 2; k <= max_k; k++)
        {
            // Массив меток кластеров для каждой точки
            int *labels = malloc(sizeof(int) * n_points);

            // Массив центроидов
            Point *centroids = malloc(sizeof(Point) * k);

            if (!labels || !centroids)
            {
                fprintf(stderr, "Memory allocation failed\n");
                free(labels);
                free(centroids);
                continue;
            }

            // Выделение памяти под координаты центроидов
            for (int i = 0; i < k; i++)
            {
                centroids[i].coords = malloc(sizeof(double) * dim);
                if (!centroids[i].coords)
                {
                    fprintf(stderr, "Centroid coords allocation failed\n");

                    // Освобождение уже выделенной памяти
                    for (int j = 0; j < i; j++)
                        free(centroids[j].coords);

                    free(centroids);
                    free(labels);
                    continue;
                }
            }

            // Инициализация меток (нет принадлежности)
            for (int i = 0; i < n_points; i++)
            {
                labels[i] = -1;
            }

            // Запуск алгоритма k-means
            kmeans(data, n_points, k, dim, labels, centroids, 10000);

            // Оценка качества кластеризации
            double score = compute_silhouette(data, labels, n_points, k, dim);

            // Обновление локального лучшего результата
            if (score > local_best_score)
            {
                local_best_score = score;
                local_best_k = k;
            }

            // Освобождение памяти центроидов
            for (int i = 0; i < k; i++)
            {
                free(centroids[i].coords);
            }

            free(centroids);
            free(labels);
        }

        /*
         * Критическая секция:
         * - сравниваем локальный лучший результат потока
         * - обновляем глобальный best_k при необходимости
         *
         * Это необходимо для защиты от race condition
         */
#pragma omp critical
        {
            if (local_best_score > best_score)
            {
                best_score = local_best_score;
                best_k = local_best_k;
            }
        }
    }

    return best_k;
}
