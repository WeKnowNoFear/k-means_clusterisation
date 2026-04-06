#ifndef OUTPUT_H
#define OUTPUT_H

#include "point.h"
#include "dataset.h"

/*
 * Сохраняет точки и их метки в CSV-файл.
 *
 * Формат:
 *   x,y,label
 *
 * points   — массив точек
 * labels   — метки кластеров
 * n_points — количество точек
 */
void save_clusters(const char *filename, const Point *points, const int *labels, int n_points, int dim);

/*
 * Сохраняет центроиды в CSV-файл.
 *
 * Формат:
 *   x,y
 *
 * centroids — массив центроидов
 * k         — количество кластеров
 */
void save_centroids(const char *filename, const Point *centroids, int k, int dim);

/*
 * Сохраняет метрики качества кластеризации:
 * - WCSS
 * - силуэтный коэффициент
 *
 * Для разных значений k (2..max_k)
 *
 * dataset  — исходный датасет
 * max_k    — максимальное число кластеров
 * filename — выходной CSV файл
 */
void save_metrics_csv(const Dataset *dataset, int max_k, const char *filename);

#endif