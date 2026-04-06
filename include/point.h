#ifndef POINT_H
#define POINT_H

/*
 * Точка в N-мерном пространстве.
 *
 * coords — массив координат размерности dim
 *
 * Примечание:
 * Используется динамическое выделение памяти
 * для поддержки произвольной размерности.
 */
typedef struct point
{
    double *coords;
} Point;

#endif