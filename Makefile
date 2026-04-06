# Компилятор
CC = gcc

# Флаги
CFLAGS = -Wall -Iinclude -O2 -MMD -MP -fopenmp

# Библиотеки
LIBS = -lm

# Директории
SRC_DIR = src
OBJ_DIR = build/obj
LIB_DIR = build/lib

# Исходники
SRC = $(wildcard $(SRC_DIR)/*.c)

# Объектные файлы
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Зависимости
DEPS = $(OBJ:.o=.d)

# Библиотека
LIB = $(LIB_DIR)/libkmeans.a

# Пример
EXAMPLE_SRC = examples/example.c
EXAMPLE = examples/example

# Цели 

all: $(LIB) example

# Создание библиотечных директорий
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Сборка библиотеки
$(LIB): $(OBJ) | $(LIB_DIR)
	ar rcs $@ $^

# Компиляция 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка примера
example: $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLE_SRC) -L$(LIB_DIR) -lkmeans $(LIBS) -o $(EXAMPLE)

# Подключение зависимостей
-include $(DEPS)

# Очистка
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(LIB_DIR) $(EXAMPLE)

re: fclean all

.PHONY: all clean fclean re example