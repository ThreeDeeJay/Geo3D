// Shaders.cpp : Defines the entry point for the console application.
//

#include "dll_assembler.hpp"
#include <iostream>
#include <tchar.h>

CRITICAL_SECTION gl_CS;

int gl_dumpBIN = false;
int gl_dumpRAW = false;
int gl_dumpASM = false;
float gl_separation = 0.1f;
float gl_screenSize = 55;
float gl_conv = 1.0;
bool gl_left = false;
bool gl_DXIL_if = false;
bool gl_zDepth = false;
std::filesystem::path dump_path;

vector<string> enumerateFiles(string pathName, string filter = "") {
	vector<string> files;
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	string sName = pathName;
	sName.append(filter);
	hFind = FindFirstFileA(sName.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)	{
		string fName = pathName;
		fName.append(FindFileData.cFileName);
		files.push_back(fName);
		while (FindNextFileA(hFind, &FindFileData)) {
			fName = pathName;
			fName.append(FindFileData.cFileName);
			files.push_back(fName);
		}
		FindClose(hFind);
	}
	return files;
}

int _tmain(int argc, _TCHAR* argv[])
{
	vector<string> gameNames;
	string pathName;
	vector<string> files;
	FILE* f;
	/*
	{
		auto ASM = readFile("ac2-vs.txt");
		auto ASM2 = patch(true, ASM, true, gl_conv, gl_screenSize, gl_separation);
		auto ASM3 = changeASM(true, ASM2, true, gl_conv, gl_screenSize, gl_screenSize);
		auto ASM4 = readFile("ac2-ps.txt");
		auto ASM5 = patch(true, ASM, true, gl_conv, gl_screenSize, gl_separation);
	}

	{
		auto ASM = readFile("bio-vs.txt");
		auto ASM2 = patch(false, ASM, true, gl_conv, gl_screenSize, gl_separation);
		auto ASM3 = changeASM(false, ASM2, true, gl_conv, gl_screenSize, gl_screenSize);
		auto ASM4 = readFile("bio-ps.txt");
		auto ASM5 = patch(false, ASM, true, gl_conv, gl_screenSize, gl_separation);
	}

	{
		auto ASM = readFile("SMR-vs.txt");
		auto ASM2 = patch(false, ASM, true, gl_conv, gl_screenSize, gl_separation);
		auto ASM3 = changeASM(false, ASM2, true, gl_conv, gl_screenSize, gl_screenSize);
		auto ASM4 = readFile("SMR-ps.txt");
		auto ASM5 = patch(false, ASM, true, gl_conv, gl_screenSize, gl_separation);
	}
	*/
	char gamebuffer[100000];	
	InitializeCriticalSection(&gl_CS);
	vector<string> lines;
	fopen_s(&f, "gamelist.txt", "rb");
	if (f) {
		size_t fr = ::fread(gamebuffer, 1, 100000, f);
		fclose(f);
		lines = stringToLines(gamebuffer, fr);
	}
	if (lines.size() > 0) {
		for (auto i = lines.begin(); i != lines.end(); i++) {
			gameNames.push_back(*i);
		}
	}
	for (DWORD i = 0; i < gameNames.size(); i++) {
		string gameName = gameNames[i];
		if (gameName[0] == ';')
			continue;
		cout << gameName << endl;

		pathName = gameName;
		pathName.append("\\ShaderCache\\");
		auto newFiles = enumerateFiles(pathName, "????????-??.bin");
		//auto newFiles = enumerateFiles(pathName, "????????-??.txt");
		files.insert(files.end(), newFiles.begin(), newFiles.end());
	}

//#pragma omp parallel
//#pragma omp for
	for (int i = 0; i < files.size(); i++) {
		string fileName = files[i];
		auto BIN = readFile(fileName);
		ID3DBlob* pDissassembly;
		LPCSTR error = nullptr;
		HRESULT hr = D3DDisassemble(BIN.data(), BIN.size(), 0, error, &pDissassembly);
		if (hr == S_OK) {
			auto ASM = readV(pDissassembly->GetBufferPointer(), pDissassembly->GetBufferSize());
			if (ASM.size() > 0) {
				string write = fileName + ".ASM";
				fopen_s(&f, write.c_str(), "wb");
				fwrite(ASM.data(), 1, ASM.size(), f);
				fclose(f);
			}
		}
		else {
			DWORD* dw = (DWORD*)BIN.data();
			int dwSize = BIN.size() / 4;
			vector<DWORD> temp;
			for (int i = 0; i < dwSize; i++) {
				temp.push_back(*(dw + i));
			}
			temp = changeSM2(temp, true, gl_conv, gl_screenSize, gl_separation);
			UINT8* db = (UINT8*)temp.data();
			int dbSize = temp.size() * 4;
			vector<UINT8> BIN2;
			for (int i = 0; i < dbSize; i++) {
				BIN2.push_back(*(db + i));
			}
			if (BIN2.size() > 0) {
				string write = fileName + ".BIN2";
				fopen_s(&f, write.c_str(), "wb");
				fwrite(BIN2.data(), 1, BIN2.size(), f);
				fclose(f);
			}
		}
	}
	cout << endl;
	return 0;
}
