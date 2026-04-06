#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <omp.h>
#include "../include/point.h"

/*
 * Вычисляет квадрат евклидова расстояния между двумя точками
 * Это более быстрая версия без извлечения корня
 */
double distance_sq(const Point *a, const Point *b, int dim)
{
    double sum = 0.0;

    // суммирование квадратов разностей по каждой координате
    for (int i = 0; i < dim; i++)
    {
        double d = a->coords[i] - b->coords[i];
        sum += d * d;
    }

    return sum;
}

/*
 * Вычисляет евклидово расстояние между точками
 */
double distance(const Point *a, const Point *b, int dim)
{
    return sqrt(distance_sq(a, b, dim));
}

/*
 * Вычисляет WCSS (Within-Cluster Sum of Squares)
 * — сумма квадратов расстояний точек до их центроидов
 */
double compute_wcss(const Point *data, const int *labels, const Point *centroids, int n_points, int k, int dim)
{
    double wcss = 0.0;

#pragma omp parallel for reduction(+ : wcss)
    // параллельный подсчет суммы квадратов расстояний
    for (int i = 0; i < n_points; i++)
    {
        int cluster = labels[i];

        // проверка корректности метки кластера
        if (cluster < 0 || cluster >= k)
        {
            fprintf(stderr, "Invalid cluster label %d for point %d\n", cluster, i);
            continue;
        }

        // добавление расстояния до центроида
        wcss += distance_sq(&data[i], &centroids[cluster], dim);
    }

    return wcss;
}

/*
 * Вычисляет силуэтный коэффициент (silhouette score)
 * Метрика качества кластеризации
 */
double compute_silhouette(const Point *data, const int *labels, int n_points, int k, int dim)
{
    double total = 0.0; // сумма силуэтов
    int valid = 0;      // количество валидных точек

#pragma omp parallel for reduction(+ : total, valid) schedule(guided)
    // параллельный расчет силуэта для каждой точки
    for (int i = 0; i < n_points; i++)
    {
        int ci = labels[i];

        // пропуск некорректных меток
        if (ci < 0 || ci >= k)
            continue;

        double a = 0.0;     // среднее расстояние внутри своего кластера
        double b = DBL_MAX; // минимальное расстояние до другого кластера

        // перебор всех кластеров
        for (int c = 0; c < k; c++)
        {
            double sum = 0.0;
            int count = 0;

            // вычисление среднего расстояния до точек кластера c
            for (int j = 0; j < n_points; j++)
            {
                if (labels[j] != c)
                    continue;

                if (i == j)
                    continue;

                sum += distance(&data[i], &data[j], dim);
                count++;
            }

            if (count == 0)
                continue;

            double mean = sum / count;

            // если это свой кластер — значение a
            if (c == ci)
            {
                a = mean;
            }
            else
            {
                // иначе ищем ближайший другой кластер (b)
                if (mean < b)
                    b = mean;
            }
        }

        // если нет соседнего кластера — пропуск
        if (b == DBL_MAX)
            continue;

        double max_ab = fmax(a, b);

        // защита от деления на 0
        if (max_ab <= 1e-12)
            continue;

        // формула силуэта
        double s = (b - a) / max_ab;

        total += s;
        valid++;
    }

    // среднее значение силуэта
    return (valid > 0) ? total / valid : 0.0;
}