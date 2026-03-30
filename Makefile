# --- Configuration ---
CXX = g++

# OPTIMISATION MAXIMALE : -O3, -march=native, -ffast-math et -fopenmp (multithreading)
CXXFLAGS = -Wall -Wextra -std=c++17 -O3 -march=native -ffast-math -fopenmp -MMD -MP -Iinclude

# On ajoute -fopenmp ici aussi pour que le linker associe la librairie de multithreading
LDFLAGS = -lSDL2 -fopenmp

# Dossiers
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests

# Cibles (Exécutables)
TARGET = main
TEST_TARGET = test_runner

# --- Fichiers ---
# Sources du programme principal (exclut les tests)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# NOUVEAU : On garde tout de src/ SAUF main.o pour les tests
CORE_OBJS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

# Sources des tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Dépendances (.d)
DEPS = $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# --- Règles ---

all: $(TARGET)

# Compilation de l'application principale
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Règle pour compiler et LANCER les tests
test tests: $(TEST_TARGET)
	./$(TEST_TARGET)

# Liaison de l'exécutable de test
# Note : On inclut les objets de tests ET les objets du programme (sans le main.o)
$(TEST_TARGET): $(TEST_OBJS) $(CORE_OBJS)
	$(CXX) $(TEST_OBJS) $(CORE_OBJS) -o $(TEST_TARGET) $(LDFLAGS)

# Compilation des .cpp de src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilation des .cpp de tests/
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

.PHONY: all clean test