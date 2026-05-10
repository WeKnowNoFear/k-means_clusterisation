#include <stdio.h>
#include <stdlib.h>

#include "../include/output.h"
#include "../include/kmeans.h"
#include "../include/utils.h"
#include "../include/analysis.h"

/*
 * Сохраняет точки и их метки кластеров в CSV-файл.
 * Формат: x0,x1,...,label
 */
void save_clusters(const char *filename, const Point *points, const int *labels, int n_points, int dim)
{
    // открытие файла на запись
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Cannot open output file: %s\n", filename);
        return;
    }

    // запись заголовка CSV
    for (int d = 0; d < dim; d++)
        fprintf(file, "x%d,", d);
    fprintf(file, "label\n");

    // запись всех точек и их меток
    for (int i = 0; i < n_points; i++)
    {
        for (int d = 0; d < dim; d++)
        {
            fprintf(file, "%lf", points[i].coords[d]);
            if (d < dim - 1)
                fprintf(file, ",");
        }
        fprintf(file, ",%d\n", labels[i]);
    }

    fclose(file);
}

/*
 * Сохраняет координаты центроидов кластеров в CSV-файл.
 * Формат: x0,x1,...
 */
void save_centroids(const char *filename, const Point *centroids, int k, int dim)
{
    // открытие файла
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Cannot open centroids file: %s\n", filename);
        return;
    }

    // заголовок CSV
    for (int d = 0; d < dim; d++)
        fprintf(file, "x%d,", d);
    fprintf(file, "\n");

    // запись координат каждого центроида
    for (int i = 0; i < k; i++)
    {
        for (int d = 0; d < dim; d++)
        {
            fprintf(file, "%lf", centroids[i].coords[d]);
            if (d < dim - 1)
                fprintf(file, ",");
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

/*
 * Сохраняет метрики качества кластеризации:
 * WCSS (внутрикластерная сумма квадратов) и силуэт
 * для различных значений k
 */
void save_metrics_csv(const Dataset *dataset, int max_k, const char *filename)
{
    // проверка входных данных
    if (!dataset || !dataset->points || dataset->size <= 0)
        return;

    // открытие файла
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Cannot open metrics file: %s\n", filename);
        return;
    }

    // заголовок CSV
    fprintf(file, "k,wcss,silhouette\n");

    // массивы для хранения метрик
    double *wcss_arr = malloc(sizeof(double) * (max_k + 1));
    double *sil_arr = malloc(sizeof(double) * (max_k + 1));

    // проверка выделения памяти
    if (!wcss_arr || !sil_arr)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(wcss_arr);
        free(sil_arr);
        fclose(file);
        return;
    }

    // параллельный расчет метрик для разных k
#pragma omp parallel for
    for (int k = 2; k <= max_k; k++)
    {
        // выделение памяти под метки кластеров
        int *labels = malloc(sizeof(int) * dataset->size);
        if (!labels)
            continue;

        // выделение памяти под центроиды
        Point *centroids = malloc(sizeof(Point) * k);
        if (!centroids)
        {
            free(labels);
            continue;
        }

        // выделение памяти под координаты центроидов
        for (int i = 0; i < k; i++)
        {
            centroids[i].coords = malloc(sizeof(double) * dataset->dim);
            if (!centroids[i].coords)
            {
                // освобождение уже выделенной памяти при ошибке
                for (int j = 0; j < i; j++)
                    free(centroids[j].coords);

                free(centroids);
                free(labels);
                continue;
            }
        }

        // инициализация меток
        for (int i = 0; i < dataset->size; i++)
            labels[i] = -1;

        // запуск алгоритма k-means
        kmeans(dataset->points, dataset->size, k, dataset->dim, labels, centroids, 10000);

        // вычисление метрик
        wcss_arr[k] = compute_wcss(dataset->points, labels, centroids, dataset->size, k, dataset->dim);
        sil_arr[k] = compute_silhouette(dataset->points, labels, dataset->size, k, dataset->dim);

        // освобождение памяти
        free(labels);
        for (int i = 0; i < k; i++)
            free(centroids[i].coords);
        free(centroids);
    }

    // последовательная запись результатов в файл (без гонок)
    for (int k = 2; k <= max_k; k++)
    {
        fprintf(file, "%d,%lf,%lf\n", k, wcss_arr[k], sil_arr[k]);
    }

    // освобождение массивов метрик
    free(wcss_arr);
    free(sil_arr);

    fclose(file);
}
