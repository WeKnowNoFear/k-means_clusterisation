#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/dataset.h"
#include "../include/kmeans.h"
#include "../include/utils.h"
#include "../include/output.h"
#include "../include/analysis.h"

void test_distance_sq()
{
    Point a, b;

    double ca[] = {0, 0};
    double cb[] = {3, 4};

    a.coords = ca;
    b.coords = cb;

    double result = distance_sq(&a, &b, 2);

    assert(fabs(result - 25.0) < 1e-6);
}

void test_distance()
{
    Point a, b;

    double ca[] = {0, 0};
    double cb[] = {3, 4};

    a.coords = ca;
    b.coords = cb;

    double result = distance(&a, &b, 2);

    assert(fabs(result - 5.0) < 1e-6);
}

void test_load_dataset_valid()
{
    Dataset ds;

    int result = load_dataset("test_data.csv", &ds);

    assert(result == 0);
    assert(ds.size == 8);
    assert(ds.dim == 2);

    free_dataset(&ds);
}

void test_load_dataset_bad_file()
{
    Dataset ds;

    int result = load_dataset("no_file.csv", &ds);

    assert(result == -1);
}

void test_kmeans_simple()
{
    srand(42);

    Point data[4];

    double p1[] = {1, 1};
    double p2[] = {1, 2};
    double p3[] = {10, 10};
    double p4[] = {10, 11};

    data[0].coords = p1;
    data[1].coords = p2;
    data[2].coords = p3;
    data[3].coords = p4;

    int labels[4];

    Point centroids[2];

    for (int i = 0; i < 2; i++)
        centroids[i].coords = malloc(sizeof(double) * 2);

    kmeans(data, 4, 2, 2, labels, centroids, 100);

    assert(labels[0] == labels[1]);
    assert(labels[2] == labels[3]);
    assert(labels[0] != labels[2]);

    for (int i = 0; i < 2; i++)
        free(centroids[i].coords);
}

void test_best_k()
{
    srand(42);

    Point data[10];

    // Кластер 1
    double p1[] = {1, 1};
    double p2[] = {1, 2};
    double p3[] = {2, 1};
    double p4[] = {2, 2};
    double p5[] = {1.5, 1.5};

    // Кластер 2
    double p6[] = {20, 20};
    double p7[] = {20, 21};
    double p8[] = {21, 20};
    double p9[] = {21, 21};
    double p10[] = {20.5, 20.5};

    data[0].coords = p1;
    data[1].coords = p2;
    data[2].coords = p3;
    data[3].coords = p4;
    data[4].coords = p5;

    data[5].coords = p6;
    data[6].coords = p7;
    data[7].coords = p8;
    data[8].coords = p9;
    data[9].coords = p10;

    int best = find_best_k(data, 10, 4, 2);

    assert(best == 2);
}

void test_integration_pipeline()
{
    srand(42);

    Dataset ds;

    int result = load_dataset("test_data.csv", &ds);

    assert(result == 0);
    assert(ds.size > 0);

    int k = find_best_k(ds.points, ds.size, 4, ds.dim);

    assert(k == 2);

    int *labels = malloc(sizeof(int) * ds.size);
    assert(labels != NULL);

    Point *centroids = malloc(sizeof(Point) * k);
    assert(centroids != NULL);

    for (int i = 0; i < k; i++)
    {
        centroids[i].coords = malloc(sizeof(double) * ds.dim);
        assert(centroids[i].coords != NULL);
    }

    kmeans(
        ds.points,
        ds.size,
        k,
        ds.dim,
        labels,
        centroids,
        100);

    for (int i = 0; i < ds.size; i++)
    {
        assert(labels[i] >= 0);
        assert(labels[i] < k);
    }

    double wcss = compute_wcss(
        ds.points,
        labels,
        centroids,
        ds.size,
        k,
        ds.dim);

    assert(wcss >= 0.0);

    for (int i = 0; i < k; i++)
        free(centroids[i].coords);

    free(centroids);
    free(labels);

    free_dataset(&ds);
}

int main()
{
    test_distance_sq();
    test_distance();

    test_load_dataset_valid();
    test_load_dataset_bad_file();

    test_kmeans_simple();
    test_best_k();
    test_integration_pipeline();

    printf("All tests passed!\n");

    return 0;
}
