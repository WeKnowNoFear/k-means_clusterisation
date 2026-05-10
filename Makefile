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
TEST_DIR = tests
TEST_BUILD_DIR = build/tests

# Исходники библиотеки
SRC = $(wildcard $(SRC_DIR)/*.c)

# Объекты библиотеки
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Зависимости
DEPS = $(OBJ:.o=.d)

# Библиотека
LIB = $(LIB_DIR)/libkmeans.a

# Пример
EXAMPLE_SRC = examples/example.c
EXAMPLE = examples/example

# Тесты
TEST_SRC = $(TEST_DIR)/test_main.c
TEST_EXE = $(TEST_BUILD_DIR)/run_tests


all: $(LIB) example

# Создание директорий 

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)


# Библиотека 

$(LIB): $(OBJ) | $(LIB_DIR)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


# Example 

example: $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLE_SRC) -L$(LIB_DIR) -lkmeans $(LIBS) -o $(EXAMPLE)


# Tests 

tests: $(LIB) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -L$(LIB_DIR) -lkmeans $(LIBS) -o $(TEST_EXE)

test: tests
	cd $(TEST_DIR) && ../$(TEST_EXE)


# Зависимости

-include $(DEPS)


# Clean 

clean:
	rm -rf $(OBJ_DIR) $(TEST_BUILD_DIR)

fclean: clean
	rm -rf $(LIB_DIR) $(EXAMPLE)

re: fclean all


.PHONY: all clean fclean re example tests test
