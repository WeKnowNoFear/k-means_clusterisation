#ifndef KMEANS_H
#define KMEANS_H

#include "point.h"

/*
 * Инициализация центроидов методом k-means++.
 *
 * data     — массив точек
 * n_points — количество точек
 * k        — число кластеров
 * dim      — размерность пространства
 * centroids — массив центроидов (размер k)
 *
 * Особенность:
 * - первый центроид выбирается случайно
 * - остальные — с вероятностью пропорциональной расстоянию
 */
void kmeans_init_plus_plus(const Point *data, int n_points, int k, int dim, Point *centroids);

/*
 * Основной алгоритм K-means.
 *
 * Шаги:
 * 1. Назначение точек ближайшему центроиду
 * 2. Пересчёт центроидов
 * 3. Повтор до сходимости или max_iter
 *
 * data      — массив точек
 * n_points  — количество точек
 * k         — число кластеров
 * dim       — размерность пространства
 * labels    — массив меток (размер n_points)
 * centroids — массив центроидов (размер k)
 * max_iter  — максимум итераций
 *
 * Параллелизм:
 * - возможна параллелизация вычисления расстояний
 * - пересчёт центроидов требует аккуратности (reduce / локальные массивы)
 */
void kmeans(const Point *data, int n_points, int k, int dim, int *labels, Point *centroids, int max_iter);

#endif