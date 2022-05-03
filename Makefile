MAIN_DIR = hdss
MAP_DIR = justokmap
RANK_DIR = ranking
UTIL_DIR = utils
FETCH_DIR = prefetch
PROC_DIR = process
MOVE_DIR = movement
TEST_DIR = test_init
OBJ_DIR = obj


CPPFLAGS += -I$(MAIN_DIR) -I. -isystem
CXXFLAGS += -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -std=c++17 -O3  ${_CXXFLAGS} -g
LDFLAGS += -lpthread

all :  main write_file_and_ssd_map read_file_and_ssd_map cache_test single_cache_test

clean :
	rm *.o main read_file_and_ssd_map write_file_and_ssd_map cache_test single_cache_test

run :
	./main $(var) > "logs/compaction/temp/hdss($(var)).log"

run_cache :
	./cache_test $(var) > "logs/cache/temp/hdss($(var)).log"

run_single_cache :
	./single_cache_test $(var) >> "logs/cache/temp/single/all.log"

run5 :
	./main $(var) > "logs/compaction/v5/hdss($(var)).log"

run6:
	./main $(var) > "logs/compaction/v6/hdss($(var)).log"

run7 :
	./main $(var) > "logs/compaction/v7/hdss($(var)).log"

per :
	chmod -R 777 ./

scp :
	rsync -avzu --progress /home/aoxuyang.axy/code/DRAM-SSD-Storage/ ubuntu@vlab.ustc.edu.cn:/home/ubuntu/code/DRAM-SSD-Storage/

main.o : $(MAIN_DIR)/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MAIN_DIR)/main.cpp

main: main.o cache_manager.o prefetch.o get_embeddings.o store.o eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

store.o : $(MAIN_DIR)/store.cpp $(RANK_DIR)/cache.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MAIN_DIR)/store.cpp

store: store.o eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test_map.o : $(MAP_DIR)/test_map.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MAP_DIR)/test_map.cpp

test_map: test_map.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

eviction.o : $(MOVE_DIR)/eviction.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MOVE_DIR)/eviction.cpp

eviction: eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

cache_manager.o : $(RANK_DIR)/cache_manager.cpp $(RANK_DIR)/cache.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/cache_manager.cpp

cache_manager: cache_manager.o eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

strategy.o : $(RANK_DIR)/strategy.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/strategy.cpp

strategy: strategy.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

prefetch.o : $(FETCH_DIR)/prefetch.cpp $(RANK_DIR)/cache.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(FETCH_DIR)/prefetch.cpp

prefetch: prefetch.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

get_embeddings.o : $(PROC_DIR)/get_embeddings.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(PROC_DIR)/get_embeddings.cpp

get_embeddings: get_embeddings.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

write_file_and_ssd_map.o : $(TEST_DIR)/write_file_and_ssd_map.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/write_file_and_ssd_map.cpp

write_file_and_ssd_map: write_file_and_ssd_map.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

read_file_and_ssd_map.o : $(TEST_DIR)/read_file_and_ssd_map.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TEST_DIR)/read_file_and_ssd_map.cpp

read_file_and_ssd_map: read_file_and_ssd_map.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

cache_test.o : $(RANK_DIR)/cache_test.cpp $(RANK_DIR)/cache.h $(RANK_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/cache_test.cpp

cache_test: cache_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

single_cache_test.o : $(RANK_DIR)/single_cache_test.cpp $(RANK_DIR)/single_cache.h $(RANK_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/single_cache_test.cpp

single_cache_test: single_cache_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)