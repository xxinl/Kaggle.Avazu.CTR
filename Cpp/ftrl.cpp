#include "stdafx.h"
#include "ftrl.h"

const string fheaders[NO_FEATURE] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
, "11", "12", "13", "14", "15", "16", "17", "18", "19", "20"
, "21", "22", "23", "24", "25", "26", "27", "28", "29", "30"
, "31", "32", "33", "34", "35", "36", "37", "38", "39", "40" 
, "41", "42", "43", "44", "55", "46", "47", "48", "49", "50" };


void cpp::ftrl::_gen_features(const std::vector<string>& x_raw, std::vector<std::pair<size_t, size_t>>& x, bool is_train){

	std::hash<string> hash_fn;

	//add if id is new
	size_t id_hash = hash_fn(x_raw[0]);
	std::unordered_map<size_t, std::pair<size_t, size_t>>::iterator it_id_imp_cl = _id_imp_cl.find(id_hash);
	bool is_new = it_id_imp_cl == _id_imp_cl.end();
	if (is_new){

		if (is_train){
			_id_imp_cl.insert(std::make_pair(id_hash, std::make_pair(0, 0)));
		}
	}

	int header_i = 0;
	for (int i = 0; i < x_raw.size(); ++i){

		if (i == 1 && is_train)
			continue;

		if (header_i != 1 && header_i != 5 && header_i != 8 && header_i != 10 && header_i != 11){
				
			string f = string("F") + fheaders[header_i] + "_" + x_raw[i];
			x.push_back(std::make_pair(hash_fn(f), 1));
		}

		header_i++;
	};

	//add hour
	string f = string("FHOUR") + "_" + x_raw[1 + is_train].substr(6, 2);
	x.push_back(std::make_pair(hash_fn(f), 1));


	//imp and click
	if (is_train || !is_new){

		std::pair<size_t, size_t>* imp_cl = &_id_imp_cl[id_hash];
		if (imp_cl->first != 0)
			x.push_back(std::make_pair(hash_fn("F_IMP"), imp_cl->second / imp_cl->first));

		imp_cl->first += 1;
		imp_cl->second += std::stod(x_raw[1]);
	}

	//interact
	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[4 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[6 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[7 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[9 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[12 + is_train] + "_" + x_raw[4 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[12 + is_train] + "_" + x_raw[6 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[12 + is_train] + "_" + x_raw[7 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[12 + is_train] + "_" + x_raw[9 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[13 + is_train] + "_" + x_raw[4 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[13 + is_train] + "_" + x_raw[6 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[13 + is_train] + "_" + x_raw[7 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[13 + is_train] + "_" + x_raw[9 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[12 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;

	f = string("F") + fheaders[header_i] + "_" + x_raw[3 + is_train] + "_" + x_raw[13 + is_train];
	x.push_back(std::make_pair(hash_fn(f), 1));
	header_i++;
}