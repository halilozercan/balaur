#ifndef VOTING_H_
#define VOTING_H_

#include "seq.h"
#include "hash.h"
#include "index.h"
#include "types.h"

struct kmer_enc_t {
	kmer_cipher_t hash;
	pos_cipher_t pos;
	kmer_enc_t() {};
	kmer_enc_t(kmer_cipher_t _hash, pos_cipher_t _pos): hash(_hash), pos(_pos) {};
	
	struct comp {
		bool operator()(const kmer_enc_t& a, const kmer_enc_t& b) const {
			return a.hash < b.hash || (a.hash == b.hash &&  a.pos < b.pos);
		}
	};
};

struct kmer_match_t {
	pos_cipher_t cpos;
	pos_cipher_t rpos;
	kmer_match_t() {};
	kmer_match_t(pos_cipher_t _cpos, pos_cipher_t _rpos) : cpos(_cpos), rpos(_rpos) {}; 
};

struct voting_results;

struct voting_task {
	// layout: read [ ... kmers ...]  // contig 0 // contig 1 // ....
	// read sequence (fwd or rc) is first, following by contig kmers
	kmer_cipher_t* data;
	std::vector<int> offsets; // n_contigs + 2
	std::vector<int> contig_orig_lens;
	//uint64 key1_xor_pad;
	//uint64 key2_mult_pad;
	
	// client-only data
	int rid;
	enum strand_t {FWD, RC};
	strand_t strand;
	int start;
	int end;
	
	inline int get_contig_len(const int contig_id) const {
		return contig_orig_lens[contig_id];
	}
	inline int get_contig_data_offset(const int contig_id) const {
		return offsets[contig_id + 1] ;
	}
	inline int get_contig_data_len(const int contig_id) const {
		return offsets[contig_id +2] - offsets[contig_id +1] ;
	}
	
	kmer_cipher_t* get_read() {
		return &data[0];
	}
	
	kmer_cipher_t* get_contig(const int contig_id) {
		return &data[get_contig_data_offset(contig_id)];
	}
	
	inline int get_data_len() {
		if(offsets.size() == 0) return 0;
		return offsets[offsets.size()-1];
	}
	
	void alloc_len(const int len) {
		int cur_size = get_data_len();
		if(cur_size == 0) { // allocating read space
			cur_size += get_n_kmers(len, params->k2); // dense
		} else {
			cur_size += get_n_sampled_kmers(len, params->k2, params->sampling_intv); // sparse
			contig_orig_lens.push_back(len);
		}
		offsets.push_back(cur_size);
	}
	
	static voting_task* alloc_voting_task(const int rlen, const int rid, const strand_t strand, const std::vector<ref_match_t>& contigs, const int start, const int end) {
		voting_task* task = new voting_task();
		task->alloc_len(rlen);
		task->rid = rid;
		task->strand = strand;
		task->start = start;
		task->end = end;
		for(int i = start; i < end; i++) {
			if(!contigs[i].valid) continue;
			task->alloc_len(contigs[i].len);
		}
		if(task->offsets.size() == 1) { // all the contigs were filtered out
			delete task;
			return 0;
		}
		task->data = new kmer_cipher_t[task->get_data_len()];
		return task;
	}
	
	void free() {
		delete data;
		delete this;
	}
	
	void process(voting_results& out);
};

struct voting_stats {
		float avg_score;
};

struct voting_results {
	enum TOPID {BEST, SECOND};
	int best_score[2];
	int local_pos[2];
	seq_t global_pos[2];
	int contig_id[2];
	int rid;
	bool rc;
	
	void compare_and_update(voting_results& vr) {
		for(int i = 0; i < 2; i++) {
			if(vr.best_score[i] > best_score[BEST]) {
				if(!pos_in_range(vr.local_pos[i], local_pos[BEST], 30)) {
					best_score[SECOND] = best_score[BEST] ;
					local_pos[SECOND] = local_pos[BEST] ;
				}
				// update best alignment
				best_score[BEST] = vr.best_score[i];
				local_pos[BEST] = vr.local_pos[i];
				contig_id[BEST] = vr.contig_id[i];
			} else if(vr.best_score[i]> best_score[SECOND] ) {
				if(!pos_in_range(local_pos[i],best_score[BEST], 30)) {
					best_score[SECOND]  = vr.best_score[i];
					local_pos[SECOND]  = vr.local_pos[i];
				}
			}
		}
	}
	
	void convert2global_pos(const seq_t global_offset1, const seq_t global_offset2) {
		global_pos[0] = local_pos[0] + global_offset1;
		global_pos[1] = local_pos[1] + global_offset2;
	}
};

void run_voting(const std::vector<voting_task*>& tasks, std::vector<voting_results>& results, voting_stats& stats);

#endif