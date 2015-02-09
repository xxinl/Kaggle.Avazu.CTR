// Cpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>
#include <time.h>

#include <boost/date_time.hpp>

#include "csv.h"
#include "ftrl.h"
#include "feature_select_genetic.h"
#include "sample.h"


using std::string;

#define BLOCK_SIZE 500000 //0.5m rows
#define SAMPLE_RATIO 0.2


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

double _train(cpp::ftrl& learner, bool validate, bool last_round){

	//train----------------------------
	cpp::csv train_file("C:/Workspace/Kaggle/CRT/data/train_p1.csv");

	std::vector<std::vector<string>> block_vec;
	std::vector<std::vector<string>> block_vec_copy;
	std::future<void> fut_train;

	int block_i = 0;
	double logloss = 0;
	int logloss_size = 0;
	while (train_file.read_chunk(BLOCK_SIZE, block_vec, false)) {

		std::cout << "read in block " << block_i << " @" << _get_current_dt_str() << std::endl;

		if (fut_train.valid())
		{
			std::cout << std::endl << "waiting for training to complete" << std::endl;
			fut_train.wait();
		}
		else
			std::cout << std::endl;


		if (validate){
		
			cpp::sample sampler;
			block_vec_copy.clear();
			sampler.sampling(block_vec, block_vec_copy, BLOCK_SIZE * SAMPLE_RATIO);
		}
		else
			block_vec_copy.swap(block_vec);

		std::cout << "start training block " << block_i++ << " @" << _get_current_dt_str() << std::endl;
		fut_train = std::async([&]{

			if (block_i < 60 || !validate){

				learner.train(block_vec_copy);
			}
			else if (last_round){

				logloss += learner.validate(block_vec_copy);
				logloss_size += block_vec_copy.size();
			}
		});

		block_vec.clear();
	}

	if (fut_train.valid())
	{
		std::cout << std::endl << "waiting for training to complete" << std::endl;
		fut_train.wait();
	}

	train_file.close();
	return logloss / logloss_size;
}

void _test(cpp::ftrl& learner){

	//predict------------------------
	cpp::csv test_file("C:/Workspace/Kaggle/CRT/data/test_p1.csv");
	std::ofstream submit_file("C:/Workspace/Kaggle/CRT/data/submit.csv");

	submit_file << "id,click" << std::endl;

	std::cout << "predicting" << " @" << _get_current_dt_str() << std::endl;
	std::vector<std::vector<string>> block_vec;
	std::vector<std::vector<string>> block_vec_copy;
	std::future<void> fut_test;

	int block_i = 0;
	while (test_file.read_chunk(400000, block_vec, false)) {

		std::cout << "reading in block " << block_i << " @" << _get_current_dt_str() << std::endl;

		if (fut_test.valid())
		{
			std::cout << std::endl << "waiting for predicting to complete" << std::endl;
			fut_test.wait();
		}
		else
			std::cout << std::endl;

		block_vec_copy.swap(block_vec);

		std::cout << "start predicting block " << block_i++ << " @" << _get_current_dt_str() << std::endl;
		fut_test = std::async([&]{

			for (std::vector<std::vector<string>>::iterator it(block_vec_copy.begin());
				it != block_vec_copy.end(); ++it)	{

				double p = 0;
				p = learner.predict(*it);

				if (p > 0.99) p = 1;
				if (p < 0.001) p = 0;

				submit_file << (*it)[C_ID] << "," << p << std::endl;
			}
		});

		block_vec.clear();
	}

	if (fut_test.valid())
	{
		std::cout << std::endl << "waiting for predicting to complete" << std::endl;
		fut_test.wait();
	}

	test_file.close();

	submit_file.flush();
	submit_file.close();
}

int _tmain(int argc, _TCHAR* argv[]){
	
	const bool validate = false;
	srand((unsigned int)time(NULL));

	cpp::ftrl learner;

	double logloss = 0;
	for (int i = 0; i < 5; ++i){
		
		logloss = _train(learner, validate, i == 4);
		learner.decay_alpha();
	}
	std::cout << "train log loss:" << logloss << std::endl;

	if (!validate)
		_test(learner);

	std::cout << "done" << " @" << _get_current_dt_str() << std::endl;

	//cpp::feature_select_genetic select;
	//select.optimize();
	
	string l;
	std::getline(std::cin, l);

	return 1;
}