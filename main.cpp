#include<vector>
#include<map>
#include<iostream>
#include<iterator>
#include<algorithm>
#include<chrono>

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

RawData * load_raw_data(char *file) {
	
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
	std::cerr << "[";
	for(auto & p: *s) {
		std::cerr << "[" << p.first << ", " << p.second << "],";
	}
	std::cerr << "]";
}

void dump_raw_data(RawData* r) {

	int c = 0;
	for(auto & s: *r) {
		std::cerr << "SID=" << c;
		dump_spectrum(&s);
		c++;
		std::cerr << "\n";
	}
}

void dump_index(Index *index) {
	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		std::cerr << "MZ = " << mz << ": ";
		dump_spectrum(&(*index)[mz]);
		std::cerr << "\n";
	}
}

std::vector<SID> * load_candidates(char*file) {

	std::vector<SID> * n = new std::vector<SID>;

	for(SID i = 0; i < 2; i++) {
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

Spectrum * load_query(char * file) {
	Spectrum *n = new Spectrum;
	
	MZ mz = 1;
	while (mz < MAX_MZ) {
		n->push_back(Peak(mz, 5));
		mz += 3;
	}
	return n;
	
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

std::map<SID, Spectrum> *reconstruct_candidates(Index * index, Spectrum * query, std::vector<SID> * candidate_ids) {
	
	std::map<SID, Spectrum> * reconstructed_spectra = new std::map<SID, Spectrum>;
	for(auto &sid : *candidate_ids) {
		Spectrum s;
		for(auto & query_peak: *query) {
			for(auto & bucket_peak : (*index)[query_peak.first]) {
				if (bucket_peak.first == sid) {
					s.push_back(Peak(query_peak.first, bucket_peak.second));
				}
			}
		}
		(*reconstructed_spectra)[sid] = s;
	}
	return reconstructed_spectra;
}


int main(int argc, char * argv[]) {

	if (argc != 4) {
		std::cerr << "Usage: main <raw data file> <query file> <candidates file>\n";
		exit(1);
	}

	RawData * raw_data = load_raw_data(argv[1]);
	std::cerr << "raw_data=\n";
	dump_raw_data(raw_data);

	Spectrum *query = load_query(argv[2]);
	std::cerr << "query=\n";
	dump_spectrum(query);
	std::cerr << "\n";

	std::vector<SID> * candidate_ids = load_candidates(argv[3]);

	std::cerr << "candidate_ids=\n";
	for(auto & i: *candidate_ids) {
		std::cerr << i << ", ";
	}
	std::cerr << "\n";

	// Here's where the interesting part starts

	auto index_build_start = std::chrono::high_resolution_clock::now();
	Index * index = build_index(raw_data);
	auto index_build_end = std::chrono::high_resolution_clock::now();

	delete raw_data;
		
	dump_index(index);

	auto reconstruct_start = std::chrono::high_resolution_clock::now();
	auto reconstructed_spectra = reconstruct_candidates(index, query, candidate_ids);
	auto reconstruct_end = std::chrono::high_resolution_clock::now();	
	json_reconstruction(candidate_ids, *reconstructed_spectra);

	std::cerr << "Building the index took " << (std::chrono::duration_cast<std::chrono::nanoseconds>(index_build_end - index_build_start).count()+0.0)/1e9 << " s\n";
	std::cerr << "Reconstruction took     " << (std::chrono::duration_cast<std::chrono::nanoseconds>(reconstruct_end - reconstruct_start).count()+0.0)/1e9 << " s\n";
}

