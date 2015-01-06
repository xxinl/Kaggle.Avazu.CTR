// Cpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>
#include <ppl.h>

#include <boost/date_time.hpp>

#include "csv.h"
#include "ftrl.h"


using std::string;

#define PARALLEL_SIZE 1 //also defines how many learners. i.e. one learner per thread
#define BLOCK_SIZE 500000 //0.5m rows


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

double _train(cpp::ftrl (&learner)[PARALLEL_SIZE]){

	//train----------------------------
	cpp::csv train_file("C:/Workspace/Kaggle/CRT/data/train.csv");

	std::vector<std::vector<string>> block_vec[PARALLEL_SIZE];
	std::vector<std::vector<string>> block_vec_copy[PARALLEL_SIZE];
	std::future<void> fut_train;

	int block_i = 0;
	//concurrency::combinable<double> logloss;
	double logloss = 0;
	while (train_file.read_chunk(BLOCK_SIZE, block_vec, PARALLEL_SIZE)) {

		std::cout << "read in block " << block_i << " @" << _get_current_dt_str() << std::endl;

		if (fut_train.valid())
		{
			std::cout << std::endl << "waiting for training to complete" << std::endl;
			fut_train.wait();
		}
		else
			std::cout << std::endl;

		for (int i = 0; i < PARALLEL_SIZE; i++){

			block_vec_copy[i].swap(block_vec[i]);
		}

		std::cout << "start training block " << block_i++ << " @" << _get_current_dt_str() << std::endl;
		fut_train = std::async([&]{

			//concurrency::parallel_for(size_t(0), size_t(PARALLEL_SIZE),
			//	[&learner, &block_vec_copy, &logloss](int i){

			//	logloss.local() += learner[i].train(block_vec_copy[i]);
			//});
			logloss += learner[0].train(block_vec_copy[0]);
		});

		for (int i = 0; i < PARALLEL_SIZE; i++){

			block_vec[i].clear();
		}
	}

	if (fut_train.valid())
	{
		std::cout << std::endl << "waiting for training to complete" << std::endl;
		fut_train.wait();
	}

	train_file.close();
	//return logloss.combine(std::plus<double>()) / block_i;
	return logloss / block_i;
}

void _test(cpp::ftrl (&learner)[PARALLEL_SIZE]){

	//predict------------------------
	cpp::csv test_file("C:/Workspace/Kaggle/CRT/data/test.csv");
	std::ofstream submit_file("C:/Workspace/Kaggle/CRT/data/submit.csv");

	submit_file << "id,click" << std::endl;

	std::cout << "predicting" << " @" << _get_current_dt_str() << std::endl;
	std::vector<std::vector<string>> block_vec;
	std::vector<std::vector<string>> block_vec_copy;
	std::future<void> fut_test;

	int block_i = 0;
	while (test_file.read_chunk(400000, &block_vec, 1)) {

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
				for (int i = 0; i < PARALLEL_SIZE; i++){

					p = learner[i].predict(*it, false);
				}
				submit_file << (*it)[0] << "," << p / PARALLEL_SIZE << std::endl;
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

	cpp::ftrl learner[PARALLEL_SIZE];

	double logloss = _train(learner);
	std::cout << "train log loss-" << logloss << std::endl;

	_test(learner);
	std::cout << "done" << " @" << _get_current_dt_str() << std::endl;
	return 1;
}