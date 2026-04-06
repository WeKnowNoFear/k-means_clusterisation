import numpy as np
from sklearn.datasets import make_blobs


def generate_csv_no_label(
    filename="dataset.csv",
    n_samples=1000,
    n_clusters=4,
    n_features=2,
    std=1.0,
    random_state=42
):
    # Генерация данных
    X, _ = make_blobs(
        n_samples=n_samples,
        centers=n_clusters,
        n_features=n_features,
        cluster_std=std,
        random_state=random_state
    )

    # Сохранение без label
    np.savetxt(filename, X, delimiter=",")

    print(f"Saved to {filename} (no labels)")


if __name__ == "__main__":
    generate_csv_no_label(
        filename="dataset.csv",
        n_samples=1000,
        n_clusters=5,
        n_features=128,
        std=1.4
    )