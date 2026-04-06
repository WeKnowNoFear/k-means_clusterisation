import pandas as pd
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA

# --- Загрузка данных ---
clusters = pd.read_csv("clusters.csv")
centroids = pd.read_csv("centroids.csv")
metrics = pd.read_csv("metrics.csv")

# --- Определяем размерность ---
feature_cols = [col for col in clusters.columns if col.startswith("x")]

X = clusters[feature_cols].values
centroids_X = centroids[feature_cols].values

# --- PCA ---
pca = PCA(n_components=2)
X_2d = pca.fit_transform(X)
centroids_2d = pca.transform(centroids_X)

# --- Настройка фигуры ---
plt.figure(figsize=(15,5))

# ---------------------------
# 1. Кластеры (PCA 2D)
# ---------------------------
plt.subplot(1,3,1)

plt.scatter(
    X_2d[:, 0],
    X_2d[:, 1],
    c=clusters["label"],
    cmap="tab10",
    s=50,
    alpha=0.6
)

plt.scatter(
    centroids_2d[:, 0],
    centroids_2d[:, 1],
    c="black",
    marker="*",
    s=200,
    label="Centroids"
)

plt.xlabel("PC1")
plt.ylabel("PC2")
plt.title("K-means Clusters (PCA projection)")
plt.legend()

# ---------------------------
# 2. Elbow method
# ---------------------------
plt.subplot(1,3,2)

plt.plot(
    metrics["k"],
    metrics["wcss"],
    marker="o"
)

plt.xlabel("Number of clusters (k)")
plt.ylabel("WCSS")
plt.title("Elbow Method")

plt.xticks(metrics["k"])
plt.grid(True)

# ---------------------------
# 3. Silhouette score
# ---------------------------
plt.subplot(1,3,3)

plt.plot(
    metrics["k"],
    metrics["silhouette"],
    marker="o"
)

plt.xlabel("Number of clusters (k)")
plt.ylabel("Silhouette Score")
plt.title("Silhouette Analysis")

plt.xticks(metrics["k"])
plt.grid(True)

plt.tight_layout()
plt.show()

#source venv/bin/activate