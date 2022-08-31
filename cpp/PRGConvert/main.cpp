#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <array>
#include <algorithm>

const std::array<std::string, 127> simonsBasicCommands {
  "HIRES","PLOT","LINE","BLOCK","FCHR","FCOL","FILL","RECT","ROT","DRAW",
  "CHAR","HI COL","INV","FRAC","MOVE","PLACE","UPB","UPW","LEFTW","LEFTB",
  "DOWNB","DOWNW","RIGHTB","RIGHTW","MULTI","COLOUR","MMOB","BFLASH","MOB SET","MUSIC",
  "FLASH","REPEAT","PLAY",">>","CENTRE","ENVELOPE","CGOTO","WAVE","FETCH","AT(",
  "UNTIL",">>",">>","USE",">>","GLOBAL",">>","RESET","PROC","CALL",
  "EXEC","END PROC","EXIT","END LOOP","ON KEY","DISABLE","RESUME","LOOP","DELAY",">>",
  ">>",">>",">>","SECURE","DISAPA","CIRCLE","ON ERROR","NO ERROR","LOCAL","RCOMP",
  "ELSE","RETRACE","TRACE","DIR","PAGE","DUMP","FIND","OPTION","AUTO","OLD",
  "JOY","MOD","DIV",">>","DUP","INKEY","INST","TEST","LIN","EXOR",
  "INSERT","POT","PENX",">>","PENY","SOUND","GRAPHICS","DESIGN","RLOCMOB","CMOB",
  "BCKGNDS","PAUSE","NRM","MOB OFF","OFF","ANGL","ARC","COLD","SCRSV","SCRLD",
  "TEXT","CSET","VOL","DISK","HRDCPY","KEY","PAINT","LOW COL","COPY","MERGE",
  "RENUMBER","MEM","DETECT","CHECK","DISPLAY","ERR","OUT"
};

const std::array<std::string, 76> v2BasicCommands {
  "END","FOR","NEXT","DATA","INPUT#","INPUT","DIM","READ","LET","GOTO","RUN",
  "IF","RESTORE","GOSUB","RETURN","REM","STOP","ON","WAIT","LOAD","SAVE",
  "VERIFY","DEF","POKE","PRINT#","PRINT","CONT","LIST","CLR","CMD","SYS","OPEN",
  "CLOSE","GET","NEW","TAB(","TO","FN","SPC(","THEN","NOT","STEP","+","-","*",
  "/","^","AND","OR",">","=","<","SGN","INT","ABS","USR","FRE","POS","SQR",
  "RND","LOG","EXP","COS","SIN","TAN","ATN","PEEK","LEN","STR$","VAL","ASC",
  "CHR$","LEFT$","RIGHT$","MID$","GO"
};

class PRGException : public std::exception {
  public:
  PRGException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

static void showUsage(int argc, char** argv) {
  std::cerr << "Usage: " << argv[0] << " source target" << std::endl;
}

static bool isPRG(const std::string& filename) {
  std::ifstream sourceFile{filename, std::ios::binary};
  if (sourceFile) {
    while (sourceFile.good() && !sourceFile.eof()) {
      uint16_t buffer;
      sourceFile.read((char*)&buffer, sizeof(buffer));
      return buffer == 2049;
    }
    sourceFile.close();
  } else {
    std::stringstream ss;
    ss << "unable to open input file " << filename;
    throw PRGException(ss.str());
  }
  return false;
}


static std::string parsePRGChar(uint8_t prgChar, bool simonsBasic) {
  if (simonsBasic) {
    if (prgChar >= 0x01 && prgChar <= 0x7C) {
      return simonsBasicCommands[prgChar-0x01];
    }
  } else {
    if (prgChar >= 0x80 && prgChar <= 0xCB) {
      return v2BasicCommands[prgChar-0x80];
    }
  }
  return std::string(1,char(prgChar));
}


static std::string parsePRGLine(size_t& position,
                                const std::vector<uint8_t> data) {
  std::stringstream text;

  uint16_t lineNummber = uint16_t(data[position] | (data[position+1] << 8));
  position += 2;
  
  text << lineNummber << " ";
  
  while (data[position] != 0) {
    if (data[position] == 0x64) {
      position++;
      text << parsePRGChar(data[position++], true);
    } else {
      text << parsePRGChar(data[position++], false);
    }
  }
  position += 3; // skip zero termination and adress to next line
  
  return text.str();
}

static std::vector<std::string> parsePRG(const std::string& filename) {
  std::vector<std::string> result;
  std::ifstream sourceFile{filename, std::ios::binary};
  if (sourceFile) {
    std::vector<uint8_t> data;
    uint8_t byte;
    while (sourceFile.good() && !sourceFile.eof()) {
      sourceFile.read((char*)&byte, sizeof(byte));
      data.push_back(byte);
    }
    sourceFile.close();

    size_t position{4};
    if (data.size() < 5 || data[data.size()-1] != 0 ||
        data[data.size()-2] != 0 || data[data.size()-3] != 0) {
      std::stringstream ss;
      ss << "invalid PRG input file " << filename;
      throw PRGException(ss.str());
    }
    
    do {
      result.push_back(parsePRGLine(position, data));
    } while (position < data.size()-2);
    
  } else {
    std::stringstream ss;
    ss << "unable to open input file " << filename;
    throw PRGException(ss.str());
  }
  return result;
}

static std::vector<std::string> readPlaintext(const std::string& filename) {
  std::vector<std::string> data;
  std::ifstream file{filename};
  if (file.is_open()){
    std::string line;
    while(getline(file, line)){
      data.push_back(line);
    }
    file.close();
  } else {
    std::stringstream ss;
    ss << "unable to read input file " << filename;
    throw PRGException(ss.str());
  }
  return data;
}

static void writeText(const std::string& filename,
                      const std::vector<std::string>& text) {
  
  std::ofstream file{filename};
  if (file.is_open()){
    for (const std::string& line : text) {
      file << line << "\n";
    }
    file.close();
  } else {
    std::stringstream ss;
    ss << "unable to write output file " << filename;
    throw PRGException(ss.str());
  }
}

static std::string trim(const std::string& inStr) {
  std::string str{inStr};
  str.erase(str.find_last_not_of(' ')+1);
  str.erase(0, str.find_first_not_of(' '));
  return str;
}

static uint16_t extractLineNum(std::string& line) {
  int lineNum;
  size_t pos;
  try {
    lineNum = std::stoi(line, &pos);
  } catch (const std::invalid_argument&) {
    std::stringstream ss;
    ss << "no line number found in " << line;
    throw PRGException(ss.str());
  }

  if (lineNum > 65535) {
    std::stringstream ss;
    ss << "line number too large in line: " << line;
    throw PRGException(ss.str());
  }
  
  line = trim(line.substr(pos));
  
  return uint16_t(lineNum);
}

static std::vector<uint8_t> replaceCommand(std::string& line) {
  for (size_t i = 0;i<v2BasicCommands.size();++i) {
    if (line.rfind(v2BasicCommands[i], 0) == 0) {
      line.erase(line.begin(),
                 line.begin()+long(v2BasicCommands[i].length()));
      return {uint8_t(i+0x80)};
    }
  }

  for (size_t i = 0;i<simonsBasicCommands.size();++i) {
    if (line.rfind(simonsBasicCommands[i], 0) == 0) {
      line.erase(line.begin(),
                 line.begin()+long(simonsBasicCommands[i].length()));
      return {0x64, uint8_t(i+0x01)};
    }
  }

  return {};
}

static std::vector<uint8_t> convertLineToPRG(std::string line,
                                             uint16_t& offset,
                                             uint16_t& lastLineNum) {
  
  const uint16_t lineNum = extractLineNum(line);
  
  if (lastLineNum >= lineNum) {
    std::stringstream ss;
    ss << "invalid line number " << lineNum << " before '" << line << "'";
    throw PRGException(ss.str());
  }

  std::transform(line.begin(), line.end(), line.begin(), ::toupper);
  
  std::vector<uint8_t> data;
  data.push_back(0);
  data.push_back(0);
  data.push_back(uint8_t(lineNum));
  data.push_back(uint8_t(lineNum/256));

  size_t quotationCount{0};
  while (line.length() > 0) {
    if (line[0] == '"') quotationCount++;
    
    std::vector<uint8_t> command;
    if (quotationCount%2 == 0) {
      command = replaceCommand(line);
      data.insert(data.end(), command.begin(), command.end());
    }
    
    if (command.empty()) {
      data.push_back(uint8_t(line[0]));
      line.erase(0,1);
    }
  }
  
  data.push_back(0);
  uint16_t lineOffset = uint16_t(data.size());
  offset += lineOffset;
  data[0] = uint8_t(offset);
  data[1] = uint8_t(offset/256);
  lastLineNum = lineNum;
  return data;
}

static std::vector<uint8_t> convertToPRG(const std::vector<std::string>& text) {
  std::vector<uint8_t> data;
  
  data.push_back(0x01);
  data.push_back(0x08);
  
  uint16_t offset = 2049;
  uint16_t lastLineNum = 0;
  for (const auto& line : text) {
    std::vector<uint8_t> lineData = convertLineToPRG(trim(line), offset,
                                                     lastLineNum);
    data.insert(data.end(), lineData.begin(), lineData.end());
  }
  
  data.push_back(0);
  data.push_back(0);
  
  return data;
}

static void writePRG(const std::string& filename,
                     const std::vector<std::string>& text) {
  std::ofstream file{filename, std::ios::binary};
  if (file.is_open()){
    std::vector<uint8_t> buffer = convertToPRG(text);
    file.write((char*)buffer.data(), long(buffer.size()));
    file.close();
  } else {
    std::stringstream ss;
    ss << "unable to write output file " << filename;
    throw PRGException(ss.str());
  }
}


int main(int argc, char** argv) {
  if (argc != 3) {
    showUsage(argc,argv);
    return EXIT_FAILURE;
  }
  
  const std::string source = argv[1];
  const std::string target = argv[2];
  
  try {
    if (isPRG(source)) {
      std::cout << "Converting PRG file " << source
                << " to text file " << target << std::endl;
      const std::vector<std::string> plainText = parsePRG(source);
      writeText(target, plainText);
    } else {
      std::cout << "Converting text file " << source
                << " to PRG file " << target << std::endl;
      const std::vector<std::string> plainText = readPlaintext(source);
      writePRG(target, plainText);
    }
  } catch (const PRGException& e) {
    std::cerr << "Error during conversion:\n" << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
