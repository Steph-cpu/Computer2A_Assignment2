#include <bits/stdc++.h>
using namespace std;

struct LineInfo
{
	int lineNo;
	string original;
	string code;
	int address;
	bool hasWord;
};

struct AssemblerContext
{
	vector<LineInfo> lines;
	unordered_map<string, int> symbolTable;
	vector<uint32_t> memory;
	bool hadError = false;
};

void reportError(AssemblerContext &ctx, int lineNo, const string &msg)
{
    cerr << "[ERROR] line " << lineNo << ": " << msg << "\n";
    ctx.hadError = true;
}
void ltrim(string &s)
{
	size_t p = s.find_first_not_of(" \t\r\n");
	if(p == string::npos) s.clear();
	else s.erase(0, p);
}
void rtrim(string &s)
{
	size_t p = s.find_last_not_of(" \t\r\n");
	if(p == string::npos) s.clear();
	else s.erase(p + 1);
}
void trim(string &s)
{
	rtrim(s);
	ltrim(s);
}

vector<string> splitTokens(const string &s)
{
	string token;
	vector<string> result;
	for(char c : s)
	{
		if(isspace((unsigned char)c))
		{
			if(!token.empty())
			{
				result.push_back(token);
				token.clear();
			}
		}
		else
		{
			token.push_back(c);
		}
	}
	if(!token.empty()) result.push_back(token);
	return result;
}

bool findOpcode(const string &str, int &opcode)
{
	string st = str;
	for(char &c : st) c = toupper((unsigned char)c);
	if (st == "JMP") { opcode = 0; return true; }
    if (st == "JRP") { opcode = 1; return true; }
    if (st == "LDN") { opcode = 2; return true; }
    if (st == "STO") { opcode = 3; return true; }
    if (st == "SUB") { opcode = 4; return true; }
    if (st == "CMP") { opcode = 6; return true; }
    if (st == "STP") { opcode = 7; return true; }
	return false;
}

bool parseAddressOperand(AssemblerContext &ctx, int lineNo, const string &token, int &addrOut)
{
	bool isNum = (!token.empty() && (isdigit((unsigned char)token[0]) || token[0] == '-' || token[0] == '+'));
	if(isNum)
	{
		try
		{
			long long v = stoll(token);
			if(v < 0 || v > 63)
			{
				reportError(ctx, lineNo, "Address out of range (0..31): " + token);
				return false;
			}
			addrOut = (int)v;
			return true;
		}
		catch(...)
		{
			reportError(ctx, lineNo, "Invalid numeric address: " + token);
            return false;
		}
	}
	
	auto it = ctx.symbolTable.find(token);
	if(it == ctx.symbolTable.end())
	{
		reportError(ctx, lineNo, "Unknown symbol '" + token + "' as operand");
		return false;
	}
	addrOut = it -> second;
	return true;
}

uint32_t encodeInstructionWord(int opcode, int operand, int addrMode)
{
	uint32_t w = 0;
	
	uint32_t opv = (uint32_t)operand & ((1u << 8) - 1);
	for(int bit = 0; bit < 8; bit++)
		if(opv & (1u << bit)) w |= (1u << bit);
	
	uint32_t am = (uint32_t)addrMode & ((1u << 2) - 1);
	for(int bit = 0; bit < 2; bit++)
		if(am & (1u << bit)) w |= (1u << (8 + bit));
	
	uint32_t oc = (uint32_t)opcode & ((1u << 4) - 1);
	for(int bit = 0; bit < 4; bit++)
		if(oc & (1u << bit)) w |= (1u << (13 + bit));
	
	return w;
}

uint32_t encodeVarWord(long long value)
{
	return (uint32_t)value;
}

string wordToBinaryString(uint32_t w)
{
	string s;
	s.reserve(32);
	for(int bit = 0; bit < 32; bit++)
		s.push_back((w & (1u << bit)) ? '1' : '0');
	return s;
}

void pass1(AssemblerContext &ctx)
{
	int currentAddress = 0;
	const int MAX_WORDS = 64;
	
	for(auto &l : ctx.lines)
	{
		string s = l.original;
		size_t semi = s.find(';');
		if(semi != string::npos) s = s.substr(0, semi);
		trim(s);
		
		l.code = s;
		l.address = -1;
		l.hasWord = false;
		
		if(s.empty()) continue;
		
		auto tokens = splitTokens(s);
		if(tokens.empty()) continue;
		
		size_t idx = 0;
		
		if(!tokens[idx].empty() && tokens[idx].back() == ':')
		{
			string label = tokens[idx].substr(0, tokens[idx].size() - 1);
			if(ctx.symbolTable.count(label)) reportError(ctx, l.lineNo, "Duplicate label '" + label + "'");
			else ctx.symbolTable[label] = currentAddress;
			idx++;
		}
		
		if(idx >= tokens.size()) continue;
		
		l.address = currentAddress;
		l.hasWord = true;
		currentAddress++;
		if(currentAddress > MAX_WORDS) reportError(ctx, l.lineNo, "Exceeds 64-line memory limit");
	}
}

void pass2(AssemblerContext &ctx)
{
	const int MAX_WORDS = 64;
	ctx.memory.assign(MAX_WORDS, 0u);
	for(auto &l : ctx.lines)
	{
		if(!l.hasWord) continue;
		if(l.address < 0 || l.address >= MAX_WORDS) continue;
		
		string s = l.code;
		if(s.empty()) continue;
		auto tokens = splitTokens(s);
		if(tokens.empty()) continue;
		
		size_t idx = 0;
		if(!tokens[idx].empty() && tokens[idx].back() == ':') idx++;
		if(idx >= tokens.size()) continue;
		
		string first = tokens[idx];
		string upperFirst = first;
		for(char &c : upperFirst) c = toupper((unsigned char)c);
		idx++;
		
		if(upperFirst == "VAR")
		{
			if (idx >= tokens.size())
			{
                reportError(ctx, l.lineNo,"VAR directive missing value");
                continue;
            }
			try
			{
				long long v = stoll(tokens[idx]);
				ctx.memory[l.address] = encodeVarWord(v);
				continue;
			}
			catch(...)
			{
				reportError(ctx, l.lineNo,"Invalid VAR value: " + tokens[idx]);
			}
			continue;
		}
		
		int opcode;
		if(!findOpcode(upperFirst, opcode))
		{
			reportError(ctx, l.lineNo, "Unknown instruction '" + first + "'");
			continue;
		}
		
		int operand = 0, addrMode = 0;
		
		bool needsOperand = (upperFirst == "JMP" || upperFirst=="JRP" || upperFirst=="LDN" || upperFirst=="STO" || upperFirst=="SUB");
		if(needsOperand)
		{
			if(idx >= tokens.size())
			{
                reportError(ctx, l.lineNo, "Instruction '" + first + "' missing operand");
                continue;
            }
			
			string opTok = tokens[idx];
			
			if(!opTok.empty() && opTok[0] == '#')
			{
				addrMode = 2;
				string num = opTok.substr(1);
				if(num.empty())
				{
					reportError(ctx, l.lineNo, "Immediate operand '#' must be followed by number");
                    continue;
				}
				
				try
				{
                    long long v = stoll(num);
                    operand = (int)((uint32_t)v & ((1u << 8) - 1));
                }
				catch(...)
				{
                    reportError(ctx, l.lineNo, "Invalid immediate operand: " + opTok);
                    continue;
                }
			}
			else if(!opTok.empty() && opTok[0] == '@')
			{
				addrMode = 1;
				string sym = opTok.substr(1);
				if(sym.empty())
				{
                    reportError(ctx, l.lineNo, "Indirect operand '@' must be followed by label/address");
                    continue;
                }
				
				int addr;
				if(!parseAddressOperand(ctx, l.lineNo, sym, addr)) continue;
				operand = addr;
			}
			else
			{
				addrMode = 0;
				int addr;
				if(!parseAddressOperand(ctx, l.lineNo, opTok, addr)) continue;
				operand = addr;
			}
		}
		else
		{
			if(idx < tokens.size()) cerr << "[WARN] line " << l.lineNo << ": extra tokens after '" << first << "' are ignored\n";
			operand = 0;
			addrMode = 0;	
		}
		ctx.memory[l.address] = encodeInstructionWord(opcode, operand, addrMode);
	}
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <input_asm> <output_mc>\n";
		return 1;
	}
	
	string inFile = argv[1];
	string outFile = argv[2];
	
	ifstream fin(inFile);
    if(!fin)
	{
        cerr << "Cannot open input file: " << inFile << "\n";
        return 1;
    }

    AssemblerContext ctx;

    string line;
    int lineNo = 0;
    while(getline(fin, line))
	{
        lineNo++;
        LineInfo li;
        li.original = line;
        li.lineNo   = lineNo;
        ctx.lines.push_back(li);
    }
    fin.close();

    pass1(ctx);
    if(ctx.hadError)
	{
        cerr << "Assembly aborted due to previous errors in pass 1.\n";
        return 1;
    }

    pass2(ctx);
    if(ctx.hadError)
	{
        cerr << "Assembly aborted due to previous errors in pass 2.\n";
        return 1;
    }

    ofstream fout(outFile);
    if(!fout)
	{
        cerr << "Cannot open output file for writing: " << outFile << "\n";
        return 1;
    }

    for(size_t i = 0; i < ctx.memory.size(); i++)
        fout << wordToBinaryString(ctx.memory[i]) << "\n";
    fout.close();

    cout << "Assembly successful. Machine code written to: " << outFile << "\n";
	return 0;
}