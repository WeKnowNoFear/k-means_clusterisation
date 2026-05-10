#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/dataset.h"

#define MAX_DIM 128
#define LINE_BUF 4096

/*
 * Загружает датасет из CSV-файла.
 *
 * Алгоритм работы:
 * 1. Открытие файла
 * 2. Построчное чтение (fgets)
 * 3. Очистка строки от пробелов
 * 4. Пропуск пустых и некорректных строк
 * 5. Разделение строки на токены (strtok)
 * 6. Преобразование строк в числа (strtod)
 * 7. Проверка размерности:
 *    - первая строка задаёт dim
 *    - остальные должны совпадать
 * 8. Динамическое расширение массива точек (realloc)
 * 9. Выделение памяти под координаты каждой точки
 * 10. Копирование данных
 *
 * Особенности реализации:
 * - используется strtod вместо atof (безопасный парсинг)
 * - игнорируются пробелы вокруг значений
 * - некорректные строки не приводят к падению программы
 * - защита от переполнения размерности (MAX_DIM)
 *
 * filename:
 *   путь к CSV-файлу
 *
 * dataset:
 *   структура для записи результата
 *
 * return:
 *   0  — успех
 *  -1 — ошибка (файл или память)
 */
int load_dataset(const char *filename, Dataset *dataset)
{
    if (!dataset)
        return -1;

    dataset->points = NULL;
    dataset->size = 0;
    dataset->dim = 0;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return -1;
    }

    int capacity = 128;
    int size = 0;
    int dim = -1;

    Point *points = malloc(sizeof(Point) * capacity);
    if (!points)
    {
        fclose(file);
        return -1;
    }

    char line[LINE_BUF];

    while (fgets(line, sizeof(line), file))
    {
        char *p = line;

        // Пропуск пробельных символов в начале строки
        while (isspace(*p))
            p++;

        // Пропуск строк, не начинающихся с числа (включая пустые)
        if (!isdigit(*p) && *p != '-' && *p != '.')
            continue;

        double values[MAX_DIM];
        int current_dim = 0;

        char *token = strtok(p, ",\n");

        while (token)
        {
            // Удаление пробелов слева от токена
            while (isspace(*token))
                token++;

            // Удаление пробелов справа от токена
            char *end = token + strlen(token) - 1;
            while (end > token && isspace(*end))
                *end-- = '\0';

            if (*token == '\0')
            {
                token = strtok(NULL, ",\n");
                continue;
            }

            if (current_dim >= MAX_DIM)
            {
                fprintf(stderr, "Too many dimensions\n");
                current_dim = 0;
                break;
            }

            char *endptr;
            double val = strtod(token, &endptr);

            if (endptr == token)
            {
                // Если преобразование не удалось — строка считается некорректной
                current_dim = 0;
                break;
            }

            values[current_dim++] = val;

            token = strtok(NULL, ",\n");
        }

        if (current_dim == 0)
            continue;

        if (dim == -1)
        {
            dim = current_dim;
        }
        else if (current_dim != dim)
        {
            fprintf(stderr,
                    "Skipping line (expected %d, got %d): %s",
                    dim, current_dim, line);
            continue;
        }

        if (size >= capacity)
        {
            capacity *= 2;

            Point *tmp = realloc(points, sizeof(Point) * capacity);
            if (!tmp)
            {
                for (int i = 0; i < size; i++)
                    free(points[i].coords);

                free(points);
                fclose(file);
                return -1;
            }

            points = tmp;
        }

        points[size].coords = malloc(sizeof(double) * dim);
        if (!points[size].coords)
        {
            for (int i = 0; i < size; i++)
                free(points[i].coords);

            free(points);
            fclose(file);
            return -1;
        }

        memcpy(points[size].coords, values, sizeof(double) * dim);

        size++;
    }

    fclose(file);

    if (size > 0)
    {
        Point *tmp = realloc(points, sizeof(Point) * size);
        if (tmp)
            points = tmp;
    }

    dataset->points = points;
    dataset->size = size;
    dataset->dim = (dim == -1) ? 0 : dim;

    return 0;
}

/*
 * Освобождает память, выделенную под датасет.
 *
 * После вызова:
 *   dataset->points = NULL
 *   dataset->size   = 0
 */
void free_dataset(Dataset *dataset)
{
    if (dataset && dataset->points)
    {
        // Освобождение памяти каждой точки
        for (int i = 0; i < dataset->size; i++)
        {
            free(dataset->points[i].coords);
        }

        // Освобождение массива точек
        free(dataset->points);

        dataset->points = NULL;
        dataset->size = 0;
    }
}
