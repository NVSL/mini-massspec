#include<vector>
#include<map>
#include<iostream>
#include<iterator>

typedef unsigned int SID;
typedef unsigned int Intensity;
typedef unsigned int MZ;

typedef std::pair<MZ, Intensity> Peak;
typedef std::vector<Peak> Spectrum;
typedef std::vector<Spectrum> RawData;

typedef std::pair<SID, Intensity> BucketPeak;

typedef std::vector<BucketPeak> Bucket;

typedef std::vector<Bucket> Index;

RawData * load_raw_data() {
	return NULL;
}

Index * build_index(RawData * data) {
	return NULL;
}

std::vector<SID> * load_candidates() {

	std::vector<SID> * n = new std::vector<SID>;

	for(SID i = 0; i < 10; i++) {
		n->push_back(i);
	}

	return n;
}

void dump_reconstruction(std::vector<SID> * candidates_ids,
			 std::map<SID, Spectrum> &reconstructed_spectra) {
	std::cout << "[\n";

	for(auto i = candidates_ids->begin(); i != candidates_ids->end(); i++) {
		
		std::cout << "\t{ " << "";
		std::cout << "\"" << *i << "\": " << "[\n";
		for (auto j = reconstructed_spectra[*i].begin(); j != reconstructed_spectra[*i].end(); j++) {
			std::cout << "\t\t\t[ " << j->first << ", " << j->second << " ]";
			if (std::next(j) != reconstructed_spectra[*i].end()) {
				std::cout<<  ",";
			}
			std::cout << "\n";
		}
		std::cout << "\t\t]\n";
		std::cout << "\t}";
		if (std::next(i) != candidates_ids->end()) {
			std::cout << ",";
		}
		std::cout << "\n";
	}
	
	std::cout << "]\n";
}

int main() {

	RawData * raw_data = load_raw_data();

	Index * index = build_index(raw_data);

	std::vector<SID> * candidates_ids = load_candidates();

	std::map<SID, Spectrum> reconstructed_spectra;

	for(auto i = candidates_ids->begin(); i != candidates_ids->end(); i++) {
		Spectrum s;
		s.push_back(Peak(0,10));
		reconstructed_spectra[*i] = s;
	}

	dump_reconstruction(candidates_ids, reconstructed_spectra);
}

