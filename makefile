
SHELL := /bin/bash

# Compiler and flags
CXX = g++
CXXFLAGS = -g -Wall -Wextra -std=c++20 -Wno-unused-parameter -Werror -O3


# Phase 1 
P1_DIR = Phase-1
P1_OBJ_DIR= Phase-1/obj
P1_SRCS := $(wildcard $(P1_DIR)/*.cpp)
P1_OBJS = $(P1_SRCS:$(P1_DIR)/%.cpp=$(P1_OBJ_DIR)/%.o)



phase1:  $(P1_OBJS) P1_generate_testcases
	@$(CXX) $(CXXFLAGS) -o phase1 $(P1_OBJS)
	@echo "Phase 1 executable successfully built"
$(P1_OBJ_DIR)/%.o: $(P1_DIR)/%.cpp $(wildcard $(P1_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
P1_generate_testcases:
	@python3 $(P1_DIR)/testcase_generator.py


# Phase 2
P2_DIR = Phase-2
P2_OBJ_DIR= Phase-2/obj
P2_SRCS := $(wildcard $(P2_DIR)/*.cpp)
P2_OBJS = $(P2_SRCS:$(P2_DIR)/%.cpp=$(P2_OBJ_DIR)/%.o)


phase2:  $(P2_OBJS) P2_generate_testcases
	@$(CXX) $(CXXFLAGS) -o phase2 $(P2_OBJS)
	@echo "Phase 2 executable successfully built"
$(P2_OBJ_DIR)/%.o: $(P2_DIR)/%.cpp $(wildcard $(P2_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
P2_generate_testcases:
	@python3 $(P2_DIR)/testcase_generator.py




# Phase 3

P3_DIR = Phase-3
P3_OBJ_DIR= Phase-3/obj
P3_SRCS := $(wildcard $(P3_DIR)/*.cpp)
P3_OBJS = $(P3_SRCS:$(P3_DIR)/%.cpp=$(P3_OBJ_DIR)/%.o)

phase3: $(P3_OBJS) P3_generate_testcases
	@$(CXX) $(CXXFLAGS) -o phase3 $(P3_OBJS)
	@echo "Phase 3 executable successfully built"
$(P3_OBJ_DIR)/%.o: $(P3_DIR)/%.cpp $(wildcard $(P3_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
P3_generate_testcases:
	@python3 $(P3_DIR)/testcase_generator.py




phase1_notest:  $(P1_OBJS)
	@$(CXX) $(CXXFLAGS) -o phase1 $(P1_OBJS)
	@echo "Phase 1 executable successfully built without testcases"
$(P1_OBJ_DIR)/%.o: $(P1_DIR)/%.cpp $(wildcard $(P1_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@


phase2_notest:  $(P2_OBJS) 
	@$(CXX) $(CXXFLAGS) -o phase2 $(P2_OBJS)
	@echo "Phase 2 executable successfully built without testcases"
$(P2_OBJ_DIR)/%.o: $(P2_DIR)/%.cpp $(wildcard $(P2_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@


phase3_notest: $(P3_OBJS)
	@$(CXX) $(CXXFLAGS) -o phase3 $(P3_OBJS)
	@echo "Phase 3 executable successfully built without testcases"
$(P3_OBJ_DIR)/%.o: $(P3_DIR)/%.cpp $(wildcard $(P3_DIR)/*.hpp)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean target
clean:
	rm -f phase1 phase2 phase3
	rm -f $(P1_OBJ_DIR)/*
	rm -f $(P2_OBJ_DIR)/*
	rm -f $(P3_OBJ_DIR)/*
.PHONY: phase1 phase2 phase3 clean 

