//    Copyright 2015 Rainer Gemulla
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.


/** \file
 * Implementations for env.h */

#include <log4cxx/logger.h>

#include <boost/mpi/communicator.hpp>

#include <rg/io.h>

#include <mpi2/env.h>
#include <mpi2/task.h>

namespace mpi2 {

namespace detail {

Env* theEnv = NULL;
}

void initEnv(int rank) {
	detail::theEnv = new Env(rank);
}

void destroyEnv() {
	delete detail::theEnv;
	detail::theEnv = NULL;
}

Env& env() {
	return *detail::theEnv;
}

namespace detail {

struct TaskEnvNames {
	static inline std::string id() { return "mpi2.env.TaskEnvNames"; }
	static inline void run(Channel ch, TaskInfo info) {
		ch.send(mpi2::env().vars());
	}
};

void registerEnvTasks() {
	registerTask<TaskEnvNames>(true);
	registerEnvTasksForTypes<Mpi2BuiltinTypes>();
}

}

void lsAll() {
	using rg::operator<<;

	TaskManager& tm = TaskManager::getInstance();
	boost::mpi::communicator& world = tm.world();

	// spawn tasks
	int m = world.size();
	std::vector<Channel> channels(m, UNINITIALIZED);
	tm.spawnAll<detail::TaskEnvNames>(channels);

	// collect names
	for (int p=0; p<m; p++) {
		std::vector<Env::Var> names;
		channels[p].recv(names);
		std::cout << "Rank " << p << ": " << names << std::endl;
	}
}


} // namespace mpi2

