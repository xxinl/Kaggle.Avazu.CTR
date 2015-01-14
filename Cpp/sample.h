#ifndef _SAMPLE
#define _SAMPLE

#include <string>
#include <vector>

using std::string;

namespace cpp{

	class sample {
	private:

	public:
		sample() {	}

		void sampling(const std::vector<std::vector<string>>& train_set, std::vector<std::vector<string>>& sample_set,
			size_t no_sample){
		
			size_t one_sample_take_from = train_set.size() / no_sample;
			for (int i = 0; i < no_sample; ++i){
			
				size_t si = rand() % one_sample_take_from + i * one_sample_take_from;
				sample_set.push_back(train_set[si]);
			}
		}
	};
}

#endif