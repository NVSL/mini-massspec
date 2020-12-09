#include<vector>
#include<map>
#include<iostream>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<fstream>
#include <sstream>

typedef unsigned int SID;
typedef unsigned int Intensity;
typedef unsigned int MZ;

typedef std::pair<MZ, Intensity> Peak;
typedef std::vector<Peak> Spectrum;
typedef std::vector<Spectrum> RawData;
typedef std::pair<SID, Intensity> BucketPeak;
typedef std::vector<BucketPeak> Bucket;
typedef std::vector<Bucket> Index;

static const MZ MAX_MZ = 2000;  //max mz for indexing
static const int num_buckets = 20; //# of bins per mz used for index



RawData * load_raw_data(char *file) {
    RawData * spectra = new RawData();
    unsigned int file_id;
    unsigned int num_spectra;           //total number of spectra inside the file
    std::vector< std :: pair<unsigned int, unsigned int> > position; // starting position and ending position for each spectrum

    std::ifstream in(file, std::ios::in | std::ios::binary);
    in.read((char*)&file_id, sizeof( unsigned int ));
    in.read((char*)&num_spectra, sizeof( unsigned int ));
    position.resize(num_spectra);

    if (num_spectra > 0) {
        position[0].first = num_spectra + 2; //starting offset in the file of the first spectrum
        in.read((char *) &position[0].second, sizeof(unsigned int)); //ending position of the first spectrum

        for (unsigned int spec_idx = 1; spec_idx < num_spectra; ++spec_idx) {
            position[spec_idx].first = position[spec_idx - 1].second;  //starting position
            in.read((char *) &position[spec_idx].second, sizeof(unsigned int)); //ending position
        }

        for (unsigned int spec_idx = 0; spec_idx < num_spectra; ++spec_idx) {
            in.seekg(position[spec_idx].first * 4 + 16); //setting the offset to read the spectrum
            unsigned int size = position[spec_idx].second - position[spec_idx].first - 4;
            Spectrum spectrum;
            //Populating peak info per spectrum
            for (int i = 0; i < size; ++i) {
                unsigned int peak;
                unsigned int mz;
                in.read((char *) &peak, sizeof(unsigned int));
                mz = peak >> 8;
                spectrum.push_back(Peak(mz,peak - (mz<<8)));
            }
            spectra -> push_back(spectrum);
        }
    }
    in.close();
    return spectra;
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
        if(!(*index)[mz].empty()) {
            std::cerr << "MZ = " << mz << ": ";
            dump_spectrum(&(*index)[mz]);
            std::cerr << "\n";
        }
    }
}


void json_reconstruction( std::map<SID, Spectrum> &reconstructed_spectra) {
    std::cout << "[\n";

    auto end = reconstructed_spectra.end();

    for(const auto &[sid, spectrum]:reconstructed_spectra) {

        std::cout << "\t{ " << "";
        std::cout << "\"" << sid << "\": " << "[\n";
        for (auto j = spectrum.begin(); j != spectrum.end(); j++) {
            std::cout << "\t\t\t[ " << j->first << ", " << j->second << " ]";
            if (std::next(j) != spectrum.end()) {
                std::cout<<  ",";
            }
            std::cout << "\n";
        }
        std::cout << "\t\t]\n";
        std::cout << "\t}";

        if (&sid != &reconstructed_spectra.rbegin()->first) {
            std::cout << ",";
        }

        std::cout << "\n";
    }

    std::cout << "]\n";
}


Spectrum * load_query(char*file) {
    Spectrum *n = new Spectrum;
    std::ifstream in(file);
    std::string line;

    while (std::getline(in, line)) {
        std::vector<unsigned int> lineData;
        std::stringstream lineStream(line);
        unsigned int value;
        while (lineStream >> value) {
            lineData.push_back(value);
        }
        n->push_back(Peak(lineData[0], lineData[1]));
    }
    return n;
}

Index * build_index(RawData * data) {
    Index *index = new Index;

    for(MZ mz = 0; mz < MAX_MZ; mz++) {
        index->push_back(Bucket());
    }

    unsigned int unit_frag;
    for(SID sid = 0; sid < data->size(); sid++) {
        for(auto & peak: (*data)[sid]) {
            unit_frag = peak.first/num_buckets;
            if (unit_frag < MAX_MZ) {
                (*index)[unit_frag].push_back(BucketPeak(sid, peak.second));
            }
        }
    }
    for(MZ mz = 0; mz < MAX_MZ; mz++) {
        std::sort((*index)[mz].begin(), (*index)[mz].end());
    }
    return index;
}

std::map<SID, Spectrum> *reconstruct_candidates(Index * index, Spectrum * query) {

    std::map<SID, Spectrum> * reconstructed_spectra = new std::map<SID, Spectrum>;

    for(auto & query_peak: *query) {
        unsigned int unit_q_mz = query_peak.first / num_buckets;
        for(auto & bucket_peak : (*index)[unit_q_mz]) {
            (*reconstructed_spectra)[bucket_peak.first].push_back(Peak(query_peak.first, bucket_peak.second));
        }
    }

    return reconstructed_spectra;
}


int main(int argc, char * argv[]) {

	if (argc != 3) {
		std::cerr << "Usage: main <raw data file> <query file>\n";
		exit(1);
	}

    unsigned int total_spectra;

	RawData * raw_data = load_raw_data(argv[1]);
    total_spectra = raw_data->size();
	std::cerr << "raw_data=\n";
	dump_raw_data(raw_data);

 
	Spectrum *query = load_query(argv[2]);
	std::cerr << "query=\n";
	dump_spectrum(query);
	std::cerr << "\n";


	// Here's where the interesting part starts

	auto index_build_start = std::chrono::high_resolution_clock::now();
	Index * index = build_index(raw_data);
	auto index_build_end = std::chrono::high_resolution_clock::now();

	delete raw_data;
		
	dump_index(index);

	auto reconstruct_start = std::chrono::high_resolution_clock::now();
	auto reconstructed_spectra = reconstruct_candidates(index, query);
	auto reconstruct_end = std::chrono::high_resolution_clock::now();	
	json_reconstruction(*reconstructed_spectra);

    std::cerr << "Found " << reconstructed_spectra->size() << " candidates among "<< total_spectra << " spectra \n" ;
	std::cerr << "Building the index took " << (std::chrono::duration_cast<std::chrono::nanoseconds>(index_build_end - index_build_start).count()+0.0)/1e9 << " s\n";
	std::cerr << "Reconstruction took     " << (std::chrono::duration_cast<std::chrono::nanoseconds>(reconstruct_end - reconstruct_start).count()+0.0)/1e9 << " s\n";
}

