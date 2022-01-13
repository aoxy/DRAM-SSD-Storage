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

all :  main write_file_and_ssd_map read_file_and_ssd_map

clean :
	rm *.o main read_file_and_ssd_map write_file_and_ssd_map

run :
	./main $(var) > "logs/compaction/v2/hdss($(var)).log"

per :
	chmod -R 777 ./

scp :
	rsync -avzu --progress /home/aoxuyang.axy/code/DRAM-SSD-Storage/ ubuntu@vlab.ustc.edu.cn:/home/ubuntu/code/DRAM-SSD-Storage/

main.o : $(MAIN_DIR)/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MAIN_DIR)/main.cpp

main: main.o cache_manager.o strategy.o prefetch.o get_embeddings.o eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test_map.o : $(MAP_DIR)/test_map.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MAP_DIR)/test_map.cpp

test_map: test_map.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

eviction.o : $(MOVE_DIR)/eviction.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(MOVE_DIR)/eviction.cpp

eviction: eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

cache_manager.o : $(RANK_DIR)/cache_manager.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/cache_manager.cpp

cache_manager: cache_manager.o eviction.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

strategy.o : $(RANK_DIR)/strategy.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(RANK_DIR)/strategy.cpp

strategy: strategy.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

prefetch.o : $(FETCH_DIR)/prefetch.cpp
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
