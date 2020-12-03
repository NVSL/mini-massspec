#include<vector>
#include<map>
#include<iostream>
#include<iterator>
#include<algorithm>

typedef unsigned int SID;
typedef unsigned int Intensity;
typedef unsigned int MZ;

typedef std::pair<MZ, Intensity> Peak;
typedef std::vector<Peak> Spectrum;
typedef std::vector<Spectrum> RawData;

typedef std::pair<SID, Intensity> BucketPeak;

typedef std::vector<BucketPeak> Bucket;

typedef std::vector<Bucket> Index;

static const MZ MAX_MZ = 20;

RawData * load_raw_data() {
	
	RawData * n = new RawData;

	for (unsigned int i = 2; i < 12; i++) {
		Spectrum s;
		MZ mz = 1;
		while (mz < MAX_MZ) {
			s.push_back(Peak(mz, i));
			mz += i;
		}
		n->push_back(s);
	}
	return n;
}

void dump_spectrum(Spectrum *s) {
	std::cout << "[";
	for(auto & p: *s) {
		std::cout << "[" << p.first << ", " << p.second << "],";
	}
	std::cout << "]";
}

void dump_raw_data(RawData* r) {

	int c = 0;
	for(auto & s: *r) {
		std::cout << "SID=" << c;
		dump_spectrum(&s);
		c++;
		std::cout << "\n";
	}
}

Index * build_index(RawData * data) {
	Index *index = new Index;

	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		index->push_back(Bucket());
	}
	
	
	for(SID sid = 0; sid < data->size(); sid++) {
		for(auto & peak: (*data)[sid]) {
			(*index)[peak.first].push_back(BucketPeak(sid, peak.second));
		}
	}

	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		std::sort((*index)[mz].begin(), (*index)[mz].end());
	}

	return index;
}

void dump_index(Index *index) {
	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		std::cout << "MZ = " << mz << ": ";
		dump_spectrum(&(*index)[mz]);
		std::cout << "\n";
	}
}

std::vector<SID> * load_candidates() {

	std::vector<SID> * n = new std::vector<SID>;

	for(SID i = 0; i < 1; i++) {
		n->push_back(i);
	}

	return n;
}



void json_reconstruction(std::vector<SID> * candidates_ids,
			 std::map<SID, Spectrum> &reconstructed_spectra) {
	std::cout << "[\n";

	for(auto sid = candidates_ids->begin(); sid != candidates_ids->end(); sid++) {
		
		std::cout << "\t{ " << "";
		std::cout << "\"" << *sid << "\": " << "[\n";
		for (auto j = reconstructed_spectra[*sid].begin(); j != reconstructed_spectra[*sid].end(); j++) {
			std::cout << "\t\t\t[ " << j->first << ", " << j->second << " ]";
			if (std::next(j) != reconstructed_spectra[*sid].end()) {
				std::cout<<  ",";
			}
			std::cout << "\n";
		}
		std::cout << "\t\t]\n";
		std::cout << "\t}";
		if (std::next(sid) != candidates_ids->end()) {
			std::cout << ",";
		}
		std::cout << "\n";
	}
	
	std::cout << "]\n";
}

Spectrum * load_query() {
	Spectrum *n = new Spectrum;
	
	MZ mz = 1;
	while (mz < MAX_MZ) {
		n->push_back(Peak(mz, 5));
		mz += 3;
	}
	return n;
	
}
int main() {

	RawData * raw_data = load_raw_data();

	std::cout << "raw_data=\n";
	dump_raw_data(raw_data);
	Index * index = build_index(raw_data);

	dump_index(index);
	
	Spectrum *query = load_query();
	
	std::cout << "query=\n";
	dump_spectrum(query);
	std::cout << "\n";

	std::vector<SID> * candidate_ids = load_candidates();

	std::cout << "candidate_ids=\n";
	for(auto & i: *candidate_ids) {
		std::cout << i << ", ";
	}
	std::cout << "\n";


	std::map<SID, Spectrum> reconstructed_spectra;

	for(auto &sid : *candidate_ids) {
		Spectrum s;
		for(auto & query_peak: *query) {
			for(auto & bucket_peak : (*index)[query_peak.first]) {
				if (bucket_peak.first == sid) {
					s.push_back(Peak(query_peak.first, bucket_peak.second));
				}
			}
		}
		reconstructed_spectra[sid] = s;
	}

	json_reconstruction(candidate_ids, reconstructed_spectra);
}

