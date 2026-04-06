#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Подключаем собственные модули проекта
#include "../include/dataset.h"
#include "../include/kmeans.h"
#include "../include/utils.h"
#include "../include/output.h"
#include "../include/analysis.h"

int main()
{
    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Загружаем датасет
    Dataset dataset;

    // Пытаемся загрузить данные из файла
    if (load_dataset("dataset.csv", &dataset) != 0)
    {
        fprintf(stderr, "Failed to load dataset\n");
        return 1;
    }

    // Проверка, что данные не пустые
    if (dataset.size == 0)
    {
        printf("Dataset is empty!\n");
        return 1;
    }

    // Ограничиваем максимальное количество кластеров
    // (не больше 8 и не больше размера датасета)
    int max_k = dataset.size < 8 ? dataset.size : 8;

    // Находим оптимальное k с помощью silhouette
    int best_k = find_best_k(dataset.points, dataset.size, max_k, dataset.dim);

    printf("Best k according to silhouette: %d\n", best_k);

    // Параметры
    int k = best_k;      // используем найденное оптимальное k
    int max_iter = 1000; // максимальное число итераций K-means

    // Выделяем память для меток кластеров и центроидов
    int *labels = malloc(sizeof(int) * dataset.size);
    Point *centroids = malloc(sizeof(Point) * k);

    // Проверка успешности выделения памяти
    if (!labels || !centroids)
    {
        fprintf(stderr, "Memory allocation failed!\n");
        free_dataset(&dataset);
        return 1;
    }

    // Выделяем память под координаты каждого центроида
    for (int i = 0; i < k; i++)
    {
        centroids[i].coords = malloc(sizeof(double) * dataset.dim);

        if (!centroids[i].coords)
        {
            fprintf(stderr, "Centroid allocation failed!\n");

            // Освобождаем уже выделенную память при ошибке
            for (int j = 0; j < i; j++)
                free(centroids[j].coords);

            free(centroids);
            free(labels);
            free_dataset(&dataset);
            return 1;
        }
    }

    // Инициализируем все метки как "не назначенные"
    for (int i = 0; i < dataset.size; i++)
        labels[i] = -1;

    // Запуск алгоритма K-means
    kmeans(
        dataset.points,
        dataset.size,
        k,
        dataset.dim,
        labels,
        centroids,
        max_iter);

    //  Сохраняем результаты кластеризации в CSV
    save_clusters(
        "clusters.csv",
        dataset.points,
        labels,
        dataset.size,
        dataset.dim);

    // Сохраняем координаты центроидов
    save_centroids("centroids.csv", centroids, k, dataset.dim);

    // Сохраняем метрики (WCSS и Silhouette) для разных k
    save_metrics_csv(&dataset, max_k, "metrics.csv");

    // Освобождение памяти
    free(labels);

    // Освобождаем память каждого центроида
    for (int i = 0; i < k; i++)
    {
        free(centroids[i].coords);
    }

    free(centroids);

    // Освобождаем датасет
    free_dataset(&dataset);

    printf("Clustering results saved to CSV files:\n");
    printf(" - clusters.csv\n");
    printf(" - centroids.csv\n");
    printf(" - metrics.csv (WCSS + Silhouette for k=2..%d)\n", max_k);

    return 0;
}

// export OMP_PROC_BIND=true