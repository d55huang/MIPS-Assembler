#include "kind.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <map>
// Use only the neeeded aspects of each namespace
using std::string;
using std::vector;
using std::endl;
using std::cout;
using std::cerr;
using std::cin;
using std::putchar;
using std::getline;
using std::map;
using ASM::Token;
using ASM::Lexer;

void output(int word) {
	putchar(word >> 24);
	putchar(word >> 16);
	putchar(word >> 8);
	putchar(word);
}


void print_labels(map<string, int> &sym_table) {
	map<string, int>::iterator it;
	for (it = sym_table.begin(); it != sym_table.end(); ++it) {
		string label = it->first;
		cerr << label << " " << it->second << endl;
	}
}


void check_end(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	++it2;
	if (it2 == end) {
		return;
	} else {
		string eol = "Expecting end of line, but there's more stuff.";
		cerr << "ERROR: " << eol << endl;
	}
}

int check_reg(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	if (it2 == end || (*it2)->getKind() != ASM::REGISTER) {
		cerr << "ERROR: Expected register." << endl;
		return 1;
	}
	return 0;
}


int check_comma(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	if (it2 == end || (*it2)->getKind() != ASM::COMMA) {
		cerr << "ERROR: Expected comma." << endl;
		return 1;
	}
	return 0;
}


// Operand format .word
void dotword(map<string, int> &sym_table, vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	++it2;
	if (it2 == end) {
		cerr << "ERROR: Expecting an operand after .word." << endl;
		return;
	}
	ASM::Kind kind = (*it2)->getKind();
	if (kind == ASM::INT) {
		int word = (*it2)->toInt();
		output(word);
	} else if (kind == ASM::HEXINT) {
		int word = (*it2)->toInt();
		output(word);
	} else if (kind == ASM::ID) {
		string word = (*it2)->getLexeme();
		map<string, int>::iterator labelit = sym_table.find(word);
		if (labelit == sym_table.end()) {
			cerr << "ERROR: Label " << word << " is not defined." << endl;
			return;
		} else {
			output(labelit->second);
		}
	} else {
		cerr << "ERROR: .word operand is invalid." << endl;
		return;
	}
	check_end(it2, end);
}

// Labels
void label(map<string, int> &sym_table, const Token* token, const int &loc) {
	string label = token->getLexeme();
	label.pop_back();
	map<string, int>::iterator it;
	it = sym_table.find(label);
	if (it != sym_table.end()) {
		cerr << "ERROR: Label already exists." << endl;
	} else {
		sym_table.insert({label, loc});
	}
}


// jr, jalr
void jump(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	if (opcode == "jr") {
		Template = 0x00000008;
	} else if (opcode == "jalr") {
		Template = 0x00000009;
	}
	int reg_s = (*it2)->toInt() << 21;
	output(Template | reg_s);
	check_end(it2, end);
}


// add, sub, slt, sltu
void arithmetic(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	int reg_d = (*it2)->toInt() << 11;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (check_reg(it2, end)) return;
	int reg_s = (*it2)->toInt() << 21;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (check_reg(it2, end)) return;
	int reg_t = (*it2)->toInt() << 16;
	if (opcode == "add") {
		Template = 0x00000020;
	} else if (opcode == "sub") {
		Template = 0x00000022;
	} else if (opcode == "slt") {
		Template = 0x0000002a;
	} else if (opcode == "sltu") {
		Template = 0x0000002b;
	}
	output(Template | reg_d | reg_s | reg_t);
	check_end(it2, end);
}


// beq, bne
void branch(map<string, int> &sym_table, const int &loc_counter,
		vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	int reg_s = (*it2)->toInt() << 21;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (check_reg(it2, end)) return;
	int reg_t = (*it2)->toInt() << 16;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (it2 == end) {
		cerr << "ERROR: Expected immediate operand." << endl;
		return;
	}
	int imm_op = 0;
	ASM::Kind kind = (*it2)->getKind();
	if (kind == ASM::INT) {
		imm_op = (*it2)->toInt();
		if (imm_op < -32768 || imm_op > 32767) {
			cerr << "ERROR: Integer out of range." << endl;
			return;
		}
		if (imm_op < 0) imm_op = imm_op & 0xffff;
	} else if (kind == ASM::HEXINT) {
		imm_op = (*it2)->toInt();
		if (imm_op > 0xffff || imm_op == -1) {
			cerr << "ERROR: Hex out of range." << endl;
			return;
		}
	} else if (kind == ASM::ID) {
		string label = (*it2)->getLexeme();
		map<string, int>::iterator labelit;
		labelit = sym_table.find(label);
		if (labelit == sym_table.end()) {
			cerr << "ERROR: Label does not exist." << endl;
			return;
		}
		int location = labelit->second;
		imm_op = (location - loc_counter - 4) / 4;
		if (imm_op < -32768 || imm_op > 32767) {
			cerr << "ERROR: Label out of range." << endl;
			return;
		}
		if (imm_op < 0) imm_op = imm_op & 0xffff;
	} else {
		cerr << "ERROR: Invalid input." << endl;
		return;
	}
	if (opcode == "beq") {
		Template = 0x10000000;
	} else if (opcode == "bne") {
		Template = 0x14000000;
	}
	output(Template | reg_s | reg_t | imm_op);
	check_end(it2, end);
}

// mfhi, mflo, lis
void mf_lis(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	int reg_d = (*it2)->toInt() << 11;
	if (opcode == "mfhi") {
		Template = 0x00000010;
	} else if (opcode == "mflo") {
		Template = 0x00000012;
	} else if (opcode == "lis") {
		Template = 0x00000014;
	}
	output(Template | reg_d);
	check_end(it2, end);
}


// mult, multu, div, divu
void mult_div(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	int reg_s = (*it2)->toInt() << 21;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (check_reg(it2, end)) return;
	int reg_t = (*it2)->toInt() << 16;
	if (opcode == "mult") {
		Template = 0x00000018;
	} else if (opcode == "multu") {
		Template = 0x00000019;
	} else if (opcode == "div") {
		Template = 0x0000001a;
	} else if (opcode == "divu") {
		Template = 0x0000001b;
	}
	output(Template | reg_s | reg_t);
	check_end(it2, end);
}


// lw, sw
void load_store(vector<Token*>::iterator it2, const vector<Token*>::iterator &end) {
	int Template;
	int imm_op = 0;
	string opcode = (*it2)->getLexeme();
	++it2;
	if (check_reg(it2, end)) return;
	int reg_t = (*it2)->toInt() << 16;
	++it2;
	if (check_comma(it2, end)) return;
	++it2;
	if (it2 == end) {
		cerr << "ERROR: Expected immediate operand." << endl;
		return;
	}
	ASM::Kind kind = (*it2)->getKind();
	if (kind == ASM::INT) {
		imm_op = (*it2)->toInt();
		if (imm_op < -32768 || imm_op > 32767) {
			cerr << "ERROR: Integer out of range." << endl;
			return;
		}
		if (imm_op < 0) imm_op = imm_op & 0xffff;
	} else if (kind == ASM::HEXINT) {
		imm_op = (*it2)->toInt();
		if (imm_op > 0xffff || imm_op == -1) {
			cerr << "ERROR: Hex out of range." << endl;
			return;
		}
	} else {
		cerr << "ERROR: Invalid input." << endl;
		return;
	}
	++it2;
	if (it2 == end || (*it2)->getKind() != ASM::LPAREN) {
		cerr << "ERROR: Expected '('." << endl;
		return;
	}
	++it2;
	if (check_reg(it2, end)) return;
	int reg_s = (*it2)->toInt() << 21;
	++it2;
	if (it2 == end || (*it2)->getKind() != ASM::RPAREN) {
		cerr << "ERROR: Expected ')'." << endl;
		return;
	}
	if (opcode == "lw") {
		Template = 0x8c000000;
	} else if (opcode == "sw") {
		Template = 0xac000000;
	}
	output(Template | reg_s | reg_t | imm_op);
	check_end(it2, end);
}


int main(int argc, char* argv[]){
	// Nested vector representing lines of Tokens
	// Needs to be used here to cleanup in the case
	// of an exception
	vector< vector<Token*> > tokLines;
	try{
		// Create a MIPS recognizer to tokenize
		// the input lines
		Lexer lexer;
		// Tokenize each line of the input
		string line;
		while(getline(cin,line)){
			tokLines.push_back(lexer.scan(line));
		}

		map<string, int> sym_table;
		bool nullLine = 1;
		int loc_counter = 0;

		vector<vector<Token*> >::iterator it;

		// Pass 1: Insert all labels in symbol table
		for(it = tokLines.begin(); it != tokLines.end(); ++it){

			vector<Token*>::iterator it2;

			for(it2 = it->begin(); it2 != it->end(); ++it2){
				ASM::Kind kind = (*it2)->getKind();
				if (kind == ASM::DOTWORD || kind == ASM::ID) {
					nullLine = 0;
					break;
				} else if (kind == ASM::LABEL) {
					label(sym_table, *it2, loc_counter);
					continue;
				}
			}
			if (!nullLine) loc_counter += 4;
			nullLine = 1;
		}

		// Pass 2: Loop through all tokens
		loc_counter = 0;
		for(it = tokLines.begin(); it != tokLines.end(); ++it){

			vector<Token*>::iterator it2;

			for(it2 = it->begin(); it2 != it->end(); ++it2){
				ASM::Kind kind = (*it2)->getKind();
				if (kind == ASM::DOTWORD) {
					dotword(sym_table, it2, it->end());
					nullLine = 0;
					break;
				} else if (kind == ASM::LABEL) {
					continue;
				} else if (kind == ASM::ID) {
					string opcode = (*it2)->getLexeme();
					if (opcode == "jr" || opcode == "jalr") {
						jump(it2, it->end());
					} else if (opcode == "add" || opcode == "sub" ||
							opcode == "slt" || opcode == "sltu") {
						arithmetic(it2, it->end());
					} else if (opcode == "beq" || opcode == "bne") {
						branch(sym_table, loc_counter, it2, it->end());
					} else if (opcode == "mfhi" || opcode == "mflo" ||
							opcode == "lis") {
						mf_lis(it2, it->end());
					} else if (opcode == "mult" || opcode == "multu" ||
							opcode == "div" || opcode == "divu") {
						mult_div(it2, it->end());
					} else if (opcode == "lw" || opcode == "sw") {
						load_store(it2, it->end());
					}	else {
						cerr << "ERROR: Invalid input." << endl;
						return -1;
					}
					nullLine = 0;
					break;
				} else {
					cerr << "ERROR: Invalid input." << endl;
					return -1;
				}
				//cerr << "  Token: "  << *(*it2) << endl;
			}
			if (!nullLine) loc_counter += 4;
			nullLine = 1;
		}
		//	print_labels(sym_table);

	} catch(const string& msg){
		// If an exception occurs print the message and end the program
		cerr << msg << endl;
	}
	// Delete the Tokens that have been made
	vector<vector<Token*> >::iterator it;
	for(it = tokLines.begin(); it != tokLines.end(); ++it){
		vector<Token*>::iterator it2;
		for(it2 = it->begin(); it2 != it->end(); ++it2){
			delete *it2;
		}
	}
}
