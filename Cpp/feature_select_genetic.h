#ifndef _FEATURE_SELECT
#define _FEATURE_SELECT

#include <string>
#include <vector>
#include <fstream>

#include <boost/date_time.hpp>

#include <ppl.h>
#include <concurrent_vector.h>

#include "ftrl.h"
#include "sample.h"

using std::string;

#define TRAIN_SIZE 8000000
#define VAL_SIZE 2000000

#define TRAIN_SAMPLE_SIZE 128000
#define VAL_SAMPLE_SIZE 32000

namespace cpp{

	class feature_select_genetic {
	private:
		
		const int _elite_size;
		double _mutation_rate;
		const int _max_iteration;
		const int _population_size;
		const int _feature_size;

		int _no_iter_no_change = 0; //no of iteration that top fitness hasnot change
		const int _max_iter_no_change = 32; //max no of iteration that fitness doesnt change

		std::vector<int> _f;

		typedef std::pair<double, std::vector<std::vector<int>>> CITIZEN_TYPE;

		concurrency::concurrent_vector<CITIZEN_TYPE> _population;
		concurrency::concurrent_vector<CITIZEN_TYPE> _next_generation;

		std::vector<std::vector<string>> _train_vec;
		std::vector<std::vector<string>> _validate_vec;

		std::ofstream _submit_file;

		std::vector<std::vector<int>> _get_random_citizen(){

			int f1_size = _f.size();
			std::vector<std::vector<int>> f2;

			int i = 0;
			while (i < _feature_size){

				std::vector<int> f_pair;
				int f_pair1 = rand() % f1_size;
				int f_pair2 = rand() % f1_size;

				if (f_pair1 != f_pair2){
				
					bool exist = false;
					for (int j = 0; j < f2.size(); ++j){
					
						if ((_f[f_pair1] == f2[j][0] && _f[f_pair2] == f2[j][1]) 
							|| (_f[f_pair2] == f2[j][0] && _f[f_pair1] == f2[j][1])){
						
							exist = true;
							break;
						}
					}
					
					if (exist){
						continue;
					}

					f_pair.push_back(_f[f_pair1]);
					f_pair.push_back(_f[f_pair2]);
					f2.push_back(f_pair);
					
					i++;
				}
			}

			return f2;
		}

		void _init_population(){
			
			for (int i = 0; i < _population_size; ++i){
			
				_population.push_back(std::make_pair(0, _get_random_citizen()));
			}
			_next_generation = _population;
		}

		void _sort_population(){

			std::sort(_population.begin(), _population.end(),
				[](CITIZEN_TYPE i, CITIZEN_TYPE j){
				return i.first < j.first;
			});
		}

		void _calc_population_fitness(concurrency::concurrent_vector<CITIZEN_TYPE>& population, int start_i,
			const std::vector<std::vector<string>>& train_vec, const std::vector<std::vector<string>>& validate_vec){

			concurrency::parallel_for(start_i, _population_size,
				[&population, &train_vec, &validate_vec](int i){

				cpp::sample sampler;
				std::vector<std::vector<string>> train_sample;
				sampler.sampling(train_vec, train_sample, TRAIN_SAMPLE_SIZE);
				std::vector<std::vector<string>> val_sample;
				sampler.sampling(validate_vec, val_sample, VAL_SAMPLE_SIZE);

				cpp::ftrl learner;
				learner.f2 = population[i].second;
				learner.train(train_sample);
				population[i].first = learner.validate(val_sample) / VAL_SAMPLE_SIZE;

				string o = i % 2 == 0 ? "-" : "|";
				std::cout << o;
			});

			std::cout << std::endl;
		}

		void _mate(){

			//keep elit
			for (int i = 0; i < _elite_size; ++i){

				_next_generation[i] = _population[i];
			}

			//mate/mutate the rest
			for (int i = _elite_size; i < _population_size; ++i){

				if (rand() % 100 < _mutation_rate){

					_next_generation[i].second = _get_random_citizen();
				}
				else
				{
					int i1 = rand() % _population_size;
					if (i1 >= _elite_size)
						i1 = rand() % _population_size;
					int i2 = rand() % _population_size;
					if (i2 >= _elite_size)
						i2 = rand() % _population_size;


					std::vector<std::vector<int>> mated;
					for (int j = 0; j < _feature_size; ++j){

						std::vector<int> feature = rand() % 2 == 0 ? _population[i1].second[j] : _population[i2].second[j];
						mated.push_back(feature);
					}

					_next_generation[i].first = 0;
					_next_generation[i].second = mated;
				}
			}
		}

		void _print_elite(){
			
			for (int i = 0; i < 5; ++i){

				_submit_file << _population[i].first << ",";
				std::vector<std::vector<int>> features = _population[i].second;

				for (int i = 0; i < features.size(); ++i){

					if (i != 0) _submit_file << ",";
					if (features[i][0] < features[i][1])
						_submit_file << "{" << features[i][0] << "-" << features[i][1] << "}";
					else
						_submit_file << "{" << features[i][1] << "-" << features[i][0] << "}";
				}

				_submit_file << std::endl;				
			}

			_submit_file << std::endl;
			_submit_file.flush();
		}

		string _get_current_dt_str(){

			std::stringstream s;

			const boost::posix_time::ptime now =
				boost::posix_time::second_clock::local_time();

			boost::posix_time::time_facet*const f =
				new boost::posix_time::time_facet("%H:%M:%S");
			s.imbue(std::locale(s.getloc(), f));
			s << now;

			return s.str();
		}

		void _optimize(){

			std::cout << std::endl << "initialising population" << " @" << _get_current_dt_str() << std::endl;
			_init_population();
			_calc_population_fitness(_population, 0, _train_vec, _validate_vec);
			_sort_population();
			_print_elite();

			std::cout << "initialised population with top fitness " << _population.front().first << " @" << _get_current_dt_str() << std::endl;

			for (int i = 0; i < _max_iteration; i++){

				std::cout << "starting optimization iteration " << " @" << _get_current_dt_str() << std::endl;

				_mate();

				double top_fit = _population.front().first;
				_population.swap(_next_generation);

				//_calc_population_fitness(_population, _elite_size, _train_vec, _validate_vec);
				_calc_population_fitness(_population, 0, _train_vec, _validate_vec);
				_sort_population();
				_print_elite();

				std::cout << "completed optimization iteration " << i << " with top fitness "
					<< _population.front().first << " @" << _get_current_dt_str() << std::endl;
				
				if (top_fit == _population.front().first){

					_no_iter_no_change++;
					if (_no_iter_no_change >= _max_iter_no_change){

						std::cout << "max no of iteration that top fitness hasnot change reached:" << _max_iter_no_change << " @" << _get_current_dt_str() << std::endl;
						break;
					}

					//increase mutate rate when fitness not change
					_mutation_rate = _mutation_rate * 0.9 + 10;
				}
				else{

					_no_iter_no_change = 0;
				}
			}
		}

	public:
		feature_select_genetic(double elite_rate = 0.30f, double mutation_rate = 0.20f, 
			int max_iteration = 256, int population_size = 128, int feature_size = 16) :
			_elite_size(elite_rate * population_size), _mutation_rate(mutation_rate * 100),
			_max_iteration(max_iteration), _population_size(population_size), _feature_size(feature_size),
			_submit_file("C:/Workspace/Kaggle/CRT/data/f_select.csv"){
		
			_f = { C_DAY, C_HOUR, C_C1, C_BANNER_POS, C_SITE_ID, C_SITE_CAT, C_APP_ID, C_APP_CAT,
				C_DEV_MODEL, C_DEV_TYPE, C_C14, C_C15, C_C16, C_C17, C_C18, C_C19, C_C20, C_C21 };
			
			cpp::csv train_file("C:/Workspace/Kaggle/CRT/data/train_p.csv");
			train_file.read_chunk(TRAIN_SIZE, _train_vec);
			train_file.read_chunk(VAL_SIZE, _validate_vec);
			train_file.close();
		}

		void optimize(){
			
			_optimize();

			_submit_file.close();
			std::cout << "complete @" << _get_current_dt_str() << std::endl;
		}
	};
}
#endif