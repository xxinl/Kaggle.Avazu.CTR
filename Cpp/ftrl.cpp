#include "stdafx.h"

#include "ftrl.h"


void cpp::ftrl::_gen_features(const std::vector<string>& x_raw, std::vector<std::pair<size_t, double>>& x, bool addImp){

	std::hash<string> hash_fn;

	for (int i = 0; i < f.size(); ++i){

		if (x_raw[f[i]] == "NA") continue;

		string f_str = string("F_") + std::to_string(i) + "_" + x_raw[f[i]];
		x.push_back(std::make_pair(hash_fn(f_str), 1));
	}

	for (int i = 0; i < f2.size(); ++i){

		if (x_raw[f2[i][0]] == "NA" || x_raw[f2[i][1]] == "NA"){
			
			continue;
		}

		string f_str = "F_" + std::to_string(f2[i][0]) + "_" + std::to_string(f2[i][1]) + "_"
			+ x_raw[f2[i][0]] + "_" + x_raw[f2[i][1]];

		x.push_back(std::make_pair(hash_fn(f_str), 1));
	}

	//if (addImp){

	//	//site_id imp
	//	size_t hashed_id = hash_fn(x_raw[C_SITE_ID]);
	//	std::unordered_map<size_t, double>::const_iterator it_id_imp = _id_imp.find(hashed_id);
	//	if (it_id_imp != _id_imp.end()){

	//		x.push_back(std::make_pair(hash_fn("F_SITE_IMP"), it_id_imp->second));
	//	}
	//	//app_id imp
	//	hashed_id = hash_fn(x_raw[C_APP_ID]);
	//	it_id_imp = _id_imp.find(hashed_id);
	//	if (it_id_imp != _id_imp.end()){

	//		x.push_back(std::make_pair(hash_fn("F_APP_IMP"), it_id_imp->second));
	//	}
	//}
}