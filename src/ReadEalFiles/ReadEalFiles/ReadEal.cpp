#include <iostream>
#include <vector>

#include "eax.h"
#include "EaxMan.h"
#include "ReadEal.h"

int main(int argc, char* argv[])
{
	char* filename;
	bool dynamicquery = false;
	if (argc < 2) {
		std::cout << "usage:\n";
		std::cout << " ReadEal filename.eal    ... statically analyses the eal files\n";
		std::cout << " ReadEal -d filename.eal ... additionally allows dynamic query\n";
		return EM_INVALIDID;
	}
	else filename = argv[1];
	if (argc > 2) {
		if (strcmp(argv[1], "-d") != 0)
		{
			std::cout << "usage: ReadEal -d filename.eal\n";
			return EM_INVALIDID;
		}
		dynamicquery = true;
		filename = argv[2];
	}

	EALFileContent manEALFileContent;
	if (ReadEalManually(filename, &manEALFileContent) == EM_OK) { std::cout << " ... success\n"; }
	else
	{
		std::cout << " failed!\n";
		return EM_INVALIDID;
	}

	EALFileNames namelist;
	for (auto& s : manEALFileContent.Sources) { namelist.sourceNames.push_back(s.name); }
	for (auto& m : manEALFileContent.Materials) { namelist.materialNames.push_back(m.name); }
	for (auto& e : manEALFileContent.Environments) { namelist.environmentNames.push_back(e.name); }
	for (auto& g : manEALFileContent.Geometries) { namelist.geometryNames.push_back(g.name); }

	EALFileContent eaxmanEALFileContent;
	if (ReadEalWithEaxman(filename, &eaxmanEALFileContent, &namelist, dynamicquery) == EM_OK)
	{
		if (!dynamicquery) std::cout << " ... success\n";
	}
	else
	{
		std::cout << " ... failed!\n";
		return EM_INVALIDID;
	}

	if (!dynamicquery) CompareEALFiles(&manEALFileContent, &eaxmanEALFileContent);

	return EM_OK;
}

static int ReadEalManually(char* filename, EALFileContent* ealfile)
{
	FILE* in;
	std::cout << "analyzing file manually: " << filename;
	if (fopen_s(&in, filename, "rb"))
	{
		std::cout << " - file not found";
		return EM_INVALIDID;
	}
	Chunk hd{};
	hd.read(in);
	if (hd.Equals("RIFF")) AnalyzeEal(in, hd.length, ealfile);
	fclose(in);
	return EM_OK;
}

static int ReadEalWithEaxman(char* filename, EALFileContent* ealfile, EALFileNames* namelist, bool dynamicquery)
{
	HINSTANCE eaxDLL = NULL;
	LPEAXMANAGERCREATE eaxCreate = NULL;
	LPEAXMANAGER eaxManager = NULL;
	std::cout << "analyzing file with EaxMan.dll: " << filename;

	// Attempt to load the EAXManger.dll
	eaxDLL = LoadLibrary(L"EAXMan.dll");

	if (eaxDLL != NULL)
	{
		// Attempt to retrieve the EaxManagerCreate function address. 
		eaxCreate = (LPEAXMANAGERCREATE)GetProcAddress(eaxDLL, "EaxManagerCreate");

		if (eaxCreate != NULL)
		{
			// Attempt to create the EAXManager Interface.
			if (eaxCreate(&eaxManager) != EM_OK)
			{
				eaxManager = NULL;
				return EM_INVALIDID;
			}
		}
		else return EM_INVALIDID;
	}
	else return EM_INVALIDID;

	// Attempt to load the eal file.
	if (eaxManager->LoadDataSet(filename, 0) != EM_OK) return EM_INVALIDID;

	if (EaxmanReadDataSize(eaxManager, ealfile) != EM_OK) return EM_INVALIDID;

	//Step 3: Getting the Attributes
	if (EaxmanReadListenerAttributes(eaxManager, ealfile) != EM_OK) return EM_INVALIDID;
	if (EaxmanReadSourceAttributes(eaxManager, ealfile, namelist) != EM_OK) return EM_INVALIDID;
	if (EaxmanReadMaterialAttributes(eaxManager, ealfile, namelist) != EM_OK) return EM_INVALIDID;
	if (EaxmanReadEnvironmentAttributes(eaxManager, ealfile, namelist) != EM_OK) return EM_INVALIDID;

	//Step 4: dynamic properties
	if (dynamicquery)
	{
		std::cout << " ... success\n";
		for (int i = 0; i < 3; i++) EaxmanDynamicQuery(eaxManager, ealfile, namelist->geometryNames[0].data());
	}
	// Attempt to close the eal file.
	if (eaxManager->FreeDataSet(0) != EM_OK) return EM_INVALIDID;

	return EM_OK;
}

static void EaxmanDynamicQuery(LPEAXMANAGER eaxManager, EALFileContent* ealfile, char* geometryset)
{
	// there's only one geometry in each eal, so it's sufficient so query only for one
	// we could actually skip this step because the gID therefore is always 0, but here's to check the EaxMan.dll capabilities
	long gID;
	if (eaxManager->GetGeometrySetID(geometryset, &gID) != EM_OK) return;
	// An EMPOINT struct is initialized with the listeners
	// position. If the coordinate system being used differed, 
	// here is where you would want to convert the data 
	// (e.g. swap Y and Z if needed).
	EMPOINT ep{};
	ep.fX = ReadInputFloat("x");
	ep.fY = ReadInputFloat("y");
	ep.fZ = ReadInputFloat("z");

	long lID;
	if (eaxManager->GetListenerDynamicAttributes(gID, &ep, &lID, EMFLAG_LOCKPOSITION) != EM_OK) return;
	std::cout << "query result: Environment ID: " << lID
		<< " - EAX Environment: " <<  ealfile->Environments.at(lID).EAXEnvironment
		<< "\n";

	EMPOINT epv{};
	long obstruction, occlusion;
	float obstructionLFRatio, occlusionLFRatio, occlusionRoomRatio;
	if (eaxManager->GetSourceDynamicAttributes(gID, &ep, &obstruction, &obstructionLFRatio, &occlusion, &occlusionLFRatio, &occlusionRoomRatio, &epv, 0) != EM_OK) return;
	std::cout << "              Source obstruction attribute: " << obstruction << "\n";
	std::cout << "              Source obstruction LF ratio attribute: " << obstructionLFRatio << "\n";
	std::cout << "              Source occlusion attribute: " << occlusion << "\n";
	std::cout << "              Source occlusion LF ratio attribute: " << occlusionLFRatio << "\n";
	std::cout << "              Source occlusion room ratio attribute: " << occlusionRoomRatio << "\n";
	std::cout << "\n";
}

static float ReadInputFloat(std::string name)
{
	float input;
	while (std::cout << "Enter coordinate " << name << ": " && !(std::cin >> input))
	{
		std::cin.clear(); //clear bad input flag
		std::cin.ignore(1000, '\n'); //discard input
		std::cout << "Invalid input; please re-enter.\n";
	}
	return input;
}

static int EaxmanReadEnvironmentAttributes(LPEAXMANAGER eaxManager, EALFileContent* ealfile, EALFileNames* namelist)
{
	std::vector<EALEnvironment> envs;
	for (std::string& name : namelist->environmentNames)
	{
		LPEAXLISTENERPROPERTIES EnvironmentAttributes = new EAXLISTENERPROPERTIES();
		long elPresetID = 0;
		char* ezPresetName = name.data();
		if (eaxManager->GetEnvironmentID(ezPresetName, &elPresetID) != EM_OK) return EM_INVALIDID;
		if (eaxManager->GetEnvironmentAttributes(elPresetID, EnvironmentAttributes) != EM_OK) return EM_INVALIDID;
		std::tuple<bool, bool, bool, bool, bool, bool> flags = IdentifyEAXFlags(EnvironmentAttributes->dwFlags);
		EALEnvironment entry = {
			name,
			EnvironmentAttributes->lRoom,
			EnvironmentAttributes->lRoomHF,
			EnvironmentAttributes->flRoomRolloffFactor,
			EnvironmentAttributes->flDecayTime,
			EnvironmentAttributes->flDecayHFRatio,
			EnvironmentAttributes->lReflections,
			EnvironmentAttributes->flReflectionsDelay,
			EnvironmentAttributes->lReverb,
			EnvironmentAttributes->flReverbDelay,
			EnvironmentAttributes->dwEnvironment,
			IdentifyEAXEnvironment(EnvironmentAttributes->dwEnvironment),
			EnvironmentAttributes->flEnvironmentSize,
			EnvironmentAttributes->flEnvironmentDiffusion,
			EnvironmentAttributes->flAirAbsorptionHF,
			EnvironmentAttributes->dwFlags,
			get<0>(flags),
			get<1>(flags),
			get<2>(flags),
			get<3>(flags),
			get<4>(flags),
			get<5>(flags)
		};
		envs.push_back(entry);
		if (EnvironmentAttributes) delete EnvironmentAttributes;
	}
	ealfile->Environments = envs;
	return EM_OK;
}

static int EaxmanReadMaterialAttributes(LPEAXMANAGER eaxManager, EALFileContent* ealfile, EALFileNames* namelist)
{
	std::vector<EALMaterials> mats;
	for (std::string& name : namelist->materialNames)
	{
		MATERIALATTRIBUTES MaterialAttributes;
		long mlPresetID = 0;
		char* mzPresetName = name.data();
		if (eaxManager->GetMaterialID(mzPresetName, &mlPresetID) != EM_OK) return EM_INVALIDID;
		if (eaxManager->GetMaterialAttributes(mlPresetID, &MaterialAttributes) != EM_OK) return EM_INVALIDID;
		EALMaterials entry = {
			name,
			MaterialAttributes.lLevel,
			MaterialAttributes.fLFRatio,
			MaterialAttributes.fRoomRatio,
			MaterialAttributes.dwFlags
		};
		mats.push_back(entry);
	}
	ealfile->Materials = mats;
	return EM_OK;
}

static int EaxmanReadSourceAttributes(LPEAXMANAGER eaxManager, EALFileContent* ealfile, EALFileNames* namelist)
{
	std::vector<EALSourceAttributes> srcs;
	for (std::string& name : namelist->sourceNames)
	{
		SOURCEATTRIBUTES SourceAttributes;
		long slPresetID = 0;
		char* szPresetName = name.data();
		long nrSrcs = 0;
		if (eaxManager->GetSourceID(szPresetName, &slPresetID) != EM_OK) return EM_INVALIDID;
		if (eaxManager->GetSourceAttributes(slPresetID, &SourceAttributes) != EM_OK) return EM_INVALIDID;
		if (eaxManager->GetSourceNumInstances(slPresetID, &nrSrcs) != EM_OK) return EM_INVALIDID;
		std::vector<EMPOINT> pos{};
		for (long i = 0; i < nrSrcs; i++)
		{
			EMPOINT ep{};
			if (eaxManager->GetSourceInstancePos(slPresetID, i, &ep) != EM_OK) return EM_INVALIDID;
			pos.push_back(ep);
		}
		EALSourceAttributes entry = {
			name,
			"",
			SourceAttributes.eaxAttributes,
			SourceAttributes.ulInsideConeAngle,
			SourceAttributes.ulOutsideConeAngle,
			SourceAttributes.lConeOutsideVolume,
			SourceAttributes.fConeXdir,
			SourceAttributes.fConeYdir,
			SourceAttributes.fConeZdir,
			SourceAttributes.fMinDistance,
			SourceAttributes.fMaxDistance,
			SourceAttributes.lDupCount,
			SourceAttributes.lPriority,
			nrSrcs,
			pos
		};
		srcs.push_back(entry);
	}
	ealfile->Sources = srcs;
	return EM_OK;
}

static int EaxmanReadListenerAttributes(LPEAXMANAGER eaxManager, EALFileContent* ealfile)
{
	LISTENERATTRIBUTES ListenerAttributes;
	if (eaxManager->GetListenerAttributes(&ListenerAttributes) != EM_OK) return EM_INVALIDID;
	ealfile->lisa = ListenerAttributes;
	return EM_OK;
}

static int EaxmanReadDataSize(LPEAXMANAGER eaxManager, EALFileContent* ealfile)
{
	unsigned long* datasize = new unsigned long(0);
	if (eaxManager->GetDataSetSize(datasize, 0) != EM_OK) return EM_INVALIDID;
	if (datasize) delete datasize;
	return EM_OK;
}

void AnalyzeEal(FILE* in, int length, EALFileContent* ealfile)
{
	FourCC type{};
	type.read(in);
	int count = sizeof(FourCC); //  count includes code just read
	Chunk ck{};
	bool ckanalysed;

	while (count < length) { // reads the next 8 bytes and starts analysis if it's the next chunk
		if (ck.read(in) != EM_OK) break;
		ckanalysed = false;

		if (ck.Equals("gdfm"))
		{
			AnalyzeEalGlobals(in, ck.length, "gdfm", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("lisa"))
		{
			AnalyzeEalGlobals(in, ck.length, "lisa", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("denv"))
		{
			AnalyzeEalGlobals(in, ck.length, "denv", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("dsrc"))
		{
			AnalyzeEalGlobals(in, ck.length, "dsrc", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("dfil"))
		{
			AnalyzeEalGlobals(in, ck.length, "dfil", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("dmat"))
		{
			AnalyzeEalGlobals(in, ck.length, "dmat", ealfile);
			ckanalysed = true;
		}

		if (ck.Equals("LIST"))
		{
			AnalyzeEalList(in, ck.length, ealfile);
			ckanalysed = true;
		}
		if (!ckanalysed) { ck.skip(in); } // Chunk has not been read, so skip the data

		count += ck.length + 8;
		if (ck.length & 1) count++; // account for extra odd byte
	}
}

void AnalyzeEalGlobals(FILE* in, int length, const char* field, EALFileContent* ealfile)
{
	if (std::strcmp(field, "gdfm") == 0)
	{
		EALGlobalDiffractionModel gdfm;
		fread(&gdfm, sizeof(EALGlobalDiffractionModel), 1, in);
		ealfile->gdfm = gdfm;
	}

	if (std::strcmp(field, "lisa") == 0)
	{
		LISTENERATTRIBUTES lisa;
		fread(&lisa, sizeof(LISTENERATTRIBUTES), 1, in);
		ealfile->lisa = lisa;
	}

	if (std::strcmp(field, "denv") == 0)
	{
		EAXLISTENERPROPERTIES denv;
		fread(&denv, sizeof(EAXLISTENERPROPERTIES), 1, in);
		ealfile->denv = denv;
	}

	if (std::strcmp(field, "dsrc") == 0)
	{
		SOURCEATTRIBUTES dsrc;
		fread(&dsrc, sizeof(SOURCEATTRIBUTES), 1, in);
		ealfile->dsrc = dsrc;
	}

	if (std::strcmp(field, "dfil") == 0)
	{
		char dfil[260];
		fread(&dfil, sizeof(char), 260, in);
		ealfile->dfil = dfil;
	}

	if (std::strcmp(field, "dmat") == 0)
	{
		MATERIALATTRIBUTES dmat;
		fread(&dmat, sizeof(MATERIALATTRIBUTES), 1, in);
		ealfile->dmat = dmat;
	}
}

void AnalyzeEalList(FILE* in, int length, EALFileContent* ealfile)
{
	FourCC type{};
	int count = sizeof(FourCC); //  count includes code just read
	if (length > count) type.read(in);
	else return;
	int remaining = length - count;

	if (type.Equals("envp"))
		remaining -= AnalyzeEalEnvironmentAttributes(in, ealfile);
	if (type.Equals("matp"))
		remaining -= AnalyzeEalMaterialAttributes(in, ealfile);
	if (type.Equals("srcp"))
		remaining -= AnalyzeEalSourceAttributes(in, ealfile);
	if (type.Equals("gemp"))
		remaining -= AnalyzeEalGeometryAttributes(in, ealfile);

	if (remaining > 0) AnalyzeEalList(in, remaining, ealfile);
}

int AnalyzeEalEnvironmentAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'lisp' chunk */
	Chunk lisp{};
	lisp.read(in);
	bytesread += sizeof(Chunk);
	if (!lisp.type.Equals("lisp")) return bytesread;

	std::vector<EAXLISTENERPROPERTIES> eprops;
	for (uint32_t i = 0; i < quantity; i++)
	{
		EAXLISTENERPROPERTIES eprop;
		fread(&eprop, sizeof(EAXLISTENERPROPERTIES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(EAXLISTENERPROPERTIES);
		eprops.push_back(eprop);
	}

	std::vector<EALEnvironment> data;
	for (uint32_t i = 0; i < quantity; i++)
	{
		std::tuple<bool, bool, bool, bool, bool, bool> flags = IdentifyEAXFlags(eprops[i].dwFlags);
		EALEnvironment entry = {
			names[i],
			eprops[i].lRoom,
			eprops[i].lRoomHF,
			eprops[i].flRoomRolloffFactor,
			eprops[i].flDecayTime,
			eprops[i].flDecayHFRatio,
			eprops[i].lReflections,
			eprops[i].flReflectionsDelay,
			eprops[i].lReverb,
			eprops[i].flReverbDelay,
			eprops[i].dwEnvironment,
			IdentifyEAXEnvironment(eprops[i].dwEnvironment),
			eprops[i].flEnvironmentSize,
			eprops[i].flEnvironmentDiffusion,
			eprops[i].flAirAbsorptionHF,
			eprops[i].dwFlags,
			get<0>(flags),
			get<1>(flags),
			get<2>(flags),
			get<3>(flags),
			get<4>(flags),
			get<5>(flags)
		};
		data.push_back(entry);
	}
	
	ealfile->Environments = data;

	return bytesread;
}

int AnalyzeEalMaterialAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'mata' chunk */
	/* mata has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk mata{};
	mata.read(in);
	bytesread += sizeof(Chunk);
	if (!mata.type.Equals("mata")) return bytesread;

	std::vector<MATERIALATTRIBUTES> materials;
	for (uint32_t i = 0; i < quantity; i++)
	{
		MATERIALATTRIBUTES mats;
		fread(&mats, sizeof(MATERIALATTRIBUTES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(MATERIALATTRIBUTES);
		materials.push_back(mats);
	}

	std::vector<EALMaterials> data;
	for (uint32_t i = 0; i < quantity; i++)
	{
		EALMaterials entry = {
			names[i],
			materials[i].lLevel,
			materials[i].fLFRatio,
			materials[i].fRoomRatio,
			materials[i].dwFlags
		};
		data.push_back(entry);
	}

	ealfile->Materials = data;

	return bytesread;
}

int AnalyzeEalSourceAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'fils' chunk */
	/* fils has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk fils{};
	fils.read(in);
	bytesread += sizeof(Chunk);
	if (!fils.type.Equals("fils")) return bytesread;

	std::vector<std::string> files;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char filename[260];
		fread(&filename, sizeof(char), 260, in);
		if (feof(in)) return bytesread;
		bytesread += 260 * sizeof(char);
		files.push_back(filename);
	}

	/* reading 'srca' chunk */
	Chunk srca{};
	srca.read(in);
	bytesread += sizeof(Chunk);
	if (!srca.type.Equals("srca")) return bytesread;

	std::vector<SOURCEATTRIBUTES> srcattr;
	for (uint32_t i = 0; i < quantity; i++)
	{
		SOURCEATTRIBUTES sa;
		fread(&sa, sizeof(SOURCEATTRIBUTES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(SOURCEATTRIBUTES);
		srcattr.push_back(sa);
	}

	std::vector<EALSourceAttributes> data;
	for (uint32_t i = 0; i < quantity; i++)
	{
		EALSourceAttributes entry = {
			names[i],
			files[i],
			srcattr[i].eaxAttributes,
			srcattr[i].ulInsideConeAngle,
			srcattr[i].ulOutsideConeAngle,
			srcattr[i].lConeOutsideVolume,
			srcattr[i].fConeXdir,
			srcattr[i].fConeYdir,
			srcattr[i].fConeZdir,
			srcattr[i].fMinDistance,
			srcattr[i].fMaxDistance,
			srcattr[i].lDupCount,
			srcattr[i].lPriority,
			0,
			{}
		};
		data.push_back(entry);
	}

	ealfile->Sources = data;

	return bytesread;
}

int AnalyzeEalGeometryAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'fils' chunk */
	/* fils has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk fils{};
	fils.read(in);
	bytesread += sizeof(Chunk);
	if (!fils.type.Equals("fils")) return bytesread;

	std::vector<std::string> files;
	for (uint32_t i = 0; i < quantity; i++)
	{
		char filename[260];
		fread(&filename, sizeof(char), 260, in);
		if (feof(in)) return bytesread;
		bytesread += 260 * sizeof(char);
		files.push_back(filename);
	}

	/* reading 'gema' chunk */
	Chunk gema{};
	gema.read(in);
	bytesread += sizeof(Chunk);
	if (!gema.type.Equals("gema")) return bytesread;
	std::vector<std::vector<char>> geometries;

	if (quantity == 0) {
		fseek(in, gema.length, SEEK_CUR);
		bytesread += gema.length;
		geometries.push_back({});
	}
	else {
		const size_t datalen = gema.length / quantity;

		for (uint32_t i = 0; i < quantity; i++)
		{
			std::vector<char> geom;
			for (size_t d = 0; d < datalen; d++)
			{
				char bits;
				fread(&bits, sizeof(char), 1, in);
				if (feof(in)) break;
				geom.push_back(bits);
				bytesread++;
			}
			geometries.push_back(geom);
		}
	}

	std::vector<EALGeometry> data;
	for (uint32_t i = 0; i < quantity; i++)
	{
		EALGeometry entry = {
			names[i],
			files[i],
			geometries[i]
		};
		data.push_back(entry);
	}

	ealfile->Geometries = data;

	return bytesread;
}

int Chunk::read(FILE* in)
{
	int i = 0;
	char buf[8];
	fread(buf, sizeof(char), 8, in);
	if (feof(in)) return EM_INVALIDID;
	type.code = GetNumber(buf);
	length = GetNumber(buf + 4);
	return EM_OK;
}

void Chunk::skip(FILE* in) const
{
	long nbytes;
	nbytes = (length & 1 ? length + 1 : length); // round up to even word boundary
	fseek(in, nbytes, SEEK_CUR);
}

int FourCC::read(FILE* in)
{
	fread((char*)&code, sizeof(FourCC), 1, in);
	return (feof(in) ? EM_INVALIDID : EM_OK);
}

static void CompareEALFiles(EALFileContent* manual, EALFileContent* eaxman)
{
	std::cout << "comparing data obtained from EaxMan.dll with values retrieved manuall from eal file";

	bool equal = true;
	const float threshold = 0.001f;

	if (std::fabsf(manual->lisa.fDistanceFactor - eaxman->lisa.fDistanceFactor) > threshold)
	{ std::cout << "\nmismatch: [GlobalListenerAttribute->DistanceFactor] manual: " << manual->lisa.fDistanceFactor << " EaxMan.dll: " << eaxman->lisa.fDistanceFactor; equal = false; }
	if (std::fabsf(manual->lisa.fDopplerFactor - eaxman->lisa.fDopplerFactor) > threshold)
	{ std::cout << "\nmismatch: [GlobalListenerAttribute->DopplerFactor] manual: " << manual->lisa.fDopplerFactor << " EaxMan.dll: " << eaxman->lisa.fDopplerFactor; equal = false; }
	if (std::fabsf(manual->lisa.fRolloffFactor - eaxman->lisa.fRolloffFactor) > threshold)
	{ std::cout << "\nmismatch: [GlobalListenerAttribute->RolloffFactor] manual: " << manual->lisa.fRolloffFactor << " EaxMan.dll: " << eaxman->lisa.fRolloffFactor; equal = false; }

	for (auto& e : manual->Environments)
	{
		bool found = false;
		for (auto& other : eaxman->Environments)
		{
			if (std::strcmp(e.name.c_str(), other.name.c_str()) == 0)
			{
				found = true;
				if (e.Room != other.Room)
				{ std::cout << "\nmismatch: [Environment->Room] manual: " << e.Room << " EaxMan.dll: " << other.Room; equal = false;}
				if (e.RoomHF != other.RoomHF)
				{ std::cout << "\nmismatch: [Environment->RoomHF] manual: " << e.RoomHF << " EaxMan.dll: " << other.RoomHF; equal = false;}
				if (std::fabsf(e.RoomRolloffFactor - other.RoomRolloffFactor) > threshold)
				{ std::cout << "\nmismatch: [Environment->RoomRolloffFactor] manual: " << e.RoomRolloffFactor << " EaxMan.dll: " << other.RoomRolloffFactor; equal = false;}
				if (std::fabsf(e.DecayTime - other.DecayTime) > threshold)
				{ std::cout << "\nmismatch: [Environment->DecayTime] manual: " << e.DecayTime << " EaxMan.dll: " << other.DecayTime; equal = false;}
				if (std::fabsf(e.DecayHFRatio - other.DecayHFRatio) > threshold)
				{ std::cout << "\nmismatch: [Environment->DecayHFRatio] manual: " << e.DecayHFRatio << " EaxMan.dll: " << other.DecayHFRatio; equal = false;}
				if (e.Reflections != other.Reflections)
				{ std::cout << "\nmismatch: [Environment->Reflections] manual: " << e.Reflections << " EaxMan.dll: " << other.Reflections; equal = false;}
				if (std::fabsf(e.ReflectionsDelay - other.ReflectionsDelay) > threshold)
				{ std::cout << "\nmismatch: [Environment->ReflectionsDelay] manual: " << e.ReflectionsDelay << " EaxMan.dll: " << other.ReflectionsDelay; equal = false;}
				if (e.Reverb != other.Reverb)
				{ std::cout << "\nmismatch: [Environment->Reverb] manual: " << e.Reverb << " EaxMan.dll: " << other.Reverb; equal = false;}
				if (std::fabsf(e.ReverbDelay - other.ReverbDelay) > threshold)
				{ std::cout << "\nmismatch: [Environment->ReverbDelay] manual: " << e.ReverbDelay << " EaxMan.dll: " << other.ReverbDelay; equal = false;}
				if (e.Environment != other.Environment)
				{ std::cout << "\nmismatch: [Environment->Environment] manual: " << e.Environment << " EaxMan.dll: " << other.Environment; equal = false;}
				if (std::fabsf(e.EnvironmentSize - other.EnvironmentSize) > threshold)
				{ std::cout << "\nmismatch: [Environment->EnvironmentSize] manual: " << e.EnvironmentSize << " EaxMan.dll: " << other.EnvironmentSize; equal = false;}
				if (std::fabsf(e.EnvironmentDiffusion - other.EnvironmentDiffusion) > threshold)
				{ std::cout << "\nmismatch: [Environment->EnvironmentDiffusion] manual: " << e.EnvironmentDiffusion << " EaxMan.dll: " << other.EnvironmentDiffusion; equal = false;}
				if (std::fabsf(e.AirAbsorptionHF - other.AirAbsorptionHF) > threshold)
				{ std::cout << "\nmismatch: [Environment->AirAbsorptionHF] manual: " << e.AirAbsorptionHF << " EaxMan.dll: " << other.AirAbsorptionHF; equal = false;}
				if (e.Flags != other.Flags)
				{ std::cout << "\nmismatch: [Environment->Flags] manual: " << e.Flags << " EaxMan.dll: " << other.Flags; equal = false;}
			}
		}
		if (!found) { std::cout << "\nEnvironment " << e.name << " from manual analysis not found by EaxMan.dll"; }
	}

	for (auto& e : manual->Sources)
	{
		bool found = false;
		for (auto& other : eaxman->Sources)
		{
			if (std::strcmp(e.name.c_str(), other.name.c_str()) == 0)
			{
				found = true;
				if (e.InsideConeAngle != other.InsideConeAngle)
				{ std::cout << "\nmismatch: [Sources->InsideConeAngle] manual: " << e.InsideConeAngle << " EaxMan.dll: " << other.InsideConeAngle; equal = false;}
				if (e.OutsideConeAngle != other.OutsideConeAngle)
				{ std::cout << "\nmismatch: [Sources->OutsideConeAngle] manual: " << e.OutsideConeAngle << " EaxMan.dll: " << other.OutsideConeAngle; equal = false;}
				if (e.ConeOutsideVolume != other.ConeOutsideVolume)
				{ std::cout << "\nmismatch: [Sources->ConeOutsideVolume] manual: " << e.ConeOutsideVolume << " EaxMan.dll: " << other.ConeOutsideVolume; equal = false;}
				if (std::fabsf(e.ConeXdir - other.ConeXdir) > threshold)
				{ std::cout << "\nmismatch: [Sources->ConeXdir] manual: " << e.ConeXdir << " EaxMan.dll: " << other.ConeXdir; equal = false;}
				if (std::fabsf(e.ConeYdir - other.ConeYdir) > threshold)
				{ std::cout << "\nmismatch: [Sources->ConeYdir] manual: " << e.ConeYdir << " EaxMan.dll: " << other.ConeYdir; equal = false;}
				if (std::fabsf(e.ConeZdir - other.ConeZdir) > threshold)
				{ std::cout << "\nmismatch: [Sources->ConeZdir] manual: " << e.ConeZdir << " EaxMan.dll: " << other.ConeZdir; equal = false;}
				if (std::fabsf(e.MinDistance - other.MinDistance) > threshold)
				{ std::cout << "\nmismatch: [Sources->MinDistance] manual: " << e.MinDistance << " EaxMan.dll: " << other.MinDistance; equal = false;}
				if (std::fabsf(e.MaxDistance - other.MaxDistance) > threshold)
				{ std::cout << "\nmismatch: [Sources->MaxDistance] manual: " << e.MaxDistance << " EaxMan.dll: " << other.MaxDistance; equal = false;}
				if (e.DupCount != other.DupCount)
				{ std::cout << "\nmismatch: [Sources->DupCount] manual: " << e.DupCount << " EaxMan.dll: " << other.DupCount; equal = false; }
				if (e.Priority != other.Priority)
				{ std::cout << "\nmismatch: [Sources->Priority] manual: " << e.Priority << " EaxMan.dll: " << other.Priority; equal = false;}
			}
		}
		if (!found) { std::cout << "\nSource " << e.name << " from manual analysis not found by EaxMan.dll"; }
	}

	for (auto& e : manual->Materials)
	{
		bool found = false;
		for (auto& other : eaxman->Materials)
		{
			if (std::strcmp(e.name.c_str(), other.name.c_str()) == 0)
			{
				found = true;
				if (e.Level != other.Level)
				{ std::cout << "\nmismatch: [Materials->Level] manual: " << e.Level << " EaxMan.dll: " << other.Level; equal = false; }
				if (std::fabsf(e.LFRatio - other.LFRatio) > threshold)
				{ std::cout << "\nmismatch: [Materials->LFRatio] manual: " << e.LFRatio << " EaxMan.dll: " << other.LFRatio; equal = false; }
				if (std::fabsf(e.RoomRatio - other.RoomRatio) > threshold)
				{ std::cout << "\nmismatch: [Materials->RoomRatio] manual: " << e.RoomRatio << " EaxMan.dll: " << other.RoomRatio; equal = false;}
				if (e.Flags != other.Flags)
				{ std::cout << "\nmismatch: [Materials->Flags] manual: " << e.Flags << " EaxMan.dll: " << other.Flags; equal = false; }
			}
		}
		if (!found) { std::cout << "\nMaterial " << e.name << " from manual analysis not found by EaxMan.dll"; }
	}

	if (equal) { std::cout << " ... success\ndatasets are identical\n"; }
	else { std::cout << "\ndatasets not equal!\n"; }

	std::cout << "Statistics: Geometry Sets: " << manual->Geometries.size() << " [";
	for (auto& g : manual->Geometries) { std::cout << " \"" << g.name << "\""; }
	std::cout << " ]\n";

	std::cout << "            Material Sets: " << manual->Materials.size() << " [";
	for (auto& m : manual->Materials) { std::cout << " \"" << m.name << "\""; }
	std::cout << " ]\n";

	std::cout << "            Environment Sets: " << manual->Environments.size() << " [";
	for (auto& e : manual->Environments) { std::cout << " \"" << e.name << "\""; }
	std::cout << " ]\n";

	std::cout << "            Sources Sets: " << manual->Sources.size() << " [";
	for (auto& s : manual->Sources) { std::cout << " \"" << s.name << "\""; }
	std::cout << " ]\n";

	for (size_t i = 0; i < eaxman->Sources.size(); i++)
	{
		int nr = eaxman->Sources.at(i).NrSourceInstances;
		std::cout << "            Sources placed: " << nr;
		if (nr > 0)
		{
			std::cout << " at";
			for (int k = 0; k < nr; k++)
			{
				std::cout << " (" << eaxman->Sources.at(i).LocationSourceInstances.at(k).fX << " "
					<< eaxman->Sources.at(i).LocationSourceInstances.at(k).fY << " "
					<< eaxman->Sources.at(i).LocationSourceInstances.at(k).fZ << ")\n";
			}
		}
		else std::cout << "\n";
	}
}

static std::string IdentifyEAXEnvironment(uint32_t env)
{
	switch (env)
	{
	case 0:
		return "EAX_ENVIRONMENT_GENERIC";
	case 1:
		return "EAX_ENVIRONMENT_PADDEDCELL";
	case 2:
		return "EAX_ENVIRONMENT_ROOM";
	case 3:
		return "EAX_ENVIRONMENT_BATHROOM";
	case 4:
		return "EAX_ENVIRONMENT_LIVINGROOM";
	case 5:
		return "EAX_ENVIRONMENT_STONEROOM";
	case 6:
		return "EAX_ENVIRONMENT_AUDITORIUM";
	case 7:
		return "EAX_ENVIRONMENT_CONCERTHALL";
	case 8:
		return "EAX_ENVIRONMENT_CAVE";
	case 9:
		return "EAX_ENVIRONMENT_ARENA";
	case 10:
		return "EAX_ENVIRONMENT_HANGAR";
	case 11:
		return "EAX_ENVIRONMENT_CARPETEDHALLWAY";
	case 12:
		return "EAX_ENVIRONMENT_HALLWAY";
	case 13:
		return "EAX_ENVIRONMENT_STONECORRIDOR";
	case 14:
		return "EAX_ENVIRONMENT_ALLEY";
	case 15:
		return "EAX_ENVIRONMENT_FOREST";
	case 16:
		return "EAX_ENVIRONMENT_CITY";
	case 17:
		return "EAX_ENVIRONMENT_MOUNTAINS";
	case 18:
		return "EAX_ENVIRONMENT_QUARRY";
	case 19:
		return "EAX_ENVIRONMENT_PLAIN";
	case 20:
		return "EAX_ENVIRONMENT_PARKINGLOT";
	case 21:
		return "EAX_ENVIRONMENT_SEWERPIPE";
	case 22:
		return "EAX_ENVIRONMENT_UNDERWATER";
	case 23:
		return "EAX_ENVIRONMENT_DRUGGED";
	case 24:
		return "EAX_ENVIRONMENT_DIZZY";
	case 25:
		return "EAX_ENVIRONMENT_PSYCHOTIC";
	case 26:
		return "EAX_ENVIRONMENT_COUNT";
	default:
		return "Unknown";
	}
}

static std::tuple<bool, bool, bool, bool, bool, bool> IdentifyEAXFlags(uint32_t flags)
{
	bool dts = flags & (1 << 0);
	bool rss = flags & (1 << 1);
	bool rds = flags & (1 << 2);
	bool rbs = flags & (1 << 3);
	bool bds = flags & (1 << 4);
	bool dhl = flags & (1 << 5);
	return std::tuple<bool, bool, bool, bool, bool, bool>{dts, rss, rds, rbs, bds, dhl};
}
