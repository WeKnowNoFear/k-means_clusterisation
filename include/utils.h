#ifndef UTILS_H
#define UTILS_H

#include "point.h"

/*
 * Квадрат евклидового расстояния между двумя точками.
 *
 * Используется вместо полного расстояния для:
 * - ускорения вычислений (нет sqrt)
 * - уменьшения нагрузки в k-means++
 *
 * a, b — точки
 * dim  — размерность
 */
double distance_sq(const Point *a, const Point *b, int dim);

/*
 * Евклидово расстояние между точками.
 *
 * Использует sqrt → дороже, чем distance_sq
 */
double distance(const Point *a, const Point *b, int dim);

/*
 * Вычисляет WCSS (Within-Cluster Sum of Squares).
 *
 * Формула:
 *   сумма расстояний^2 всех точек до их центроидов
 *
 * Параллелизм:
 * - распараллелен цикл по точкам
 * - используется reduction(+ : wcss)
 *
 * data      — точки
 * labels    — принадлежность к кластерам
 * centroids — центроиды
 * n_points  — количество точек
 * k         — число кластеров
 * dim       — размерность
 */
double compute_wcss(const Point *data, const int *labels, const Point *centroids, int n_points, int k, int dim);

/*
 * Силуэтный коэффициент (Silhouette Score).
 *
 * Для каждой точки:
 *   a — среднее расстояние до своего кластера
 *   b — минимальное среднее расстояние до других кластеров
 *
 * Формула:
 *   (b - a) / max(a, b)
 *
 * Параллелизм:
 * - внешний цикл распараллелен (OpenMP)
 * - reduction используется для агрегации
 *
 * Сложность: O(n²)
 *
 * data   — точки
 * labels — метки кластеров
 * n_points — количество точек
 * k      — число кластеров
 * dim    — размерность
 */
double compute_silhouette(const Point *data, const int *labels, int n_points, int k, int dim);

#endif