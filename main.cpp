/*
* Most of this code is hacky, not well though out, and could do with a revision. But hey, it wasn't serious.
*

Copyright (c) 2008 Jacob Essex

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*
*/

#include <string>
#include <sstream>
#include <fstream>

#include <vector>
#include <cassert>


#include "niflib.h"
#include "obj/NiNode.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriShapeData.h"

#include "obj/NiSourceTexture.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiMaterialProperty.h"

#include "obj/NiAVObject.h"


#ifdef _DEBUG
	#pragma comment(lib, "niflib_dll_debug.lib")
#else
	#pragma comment(lib, "niflib_dll.lib")
#endif

typedef std::list<Niflib::NiAVObjectRef> CellMeshList;

std::list<Niflib::NiAVObjectRef> gNifTri;

typedef std::vector < std::vector < float > > fHeightMap;
typedef std::map<int, std::map < int, fHeightMap > > MultiHeightMap;

MultiHeightMap gHeightmap;


//////////////////////////////////////////////////////////////////////////
//general sring functions

template <typename T>
T fromString (const std::string & s){
	T r;
	std::istringstream iss (s);
	iss >> r;
	return r;
}
template <class T>
string toString(T a){
	std::ostringstream oss;
	oss << a;
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////
//Structs for holding landscape data

struct LandSquare{
	long x;
	long y;
};

struct Normal{
	char x,y,z;
};

struct Land{
	Land(){
		land.resize(65); normals.resize(65);
		for ( int x = 0; x < 65; ++x ){
			land[x].resize(65); normals[x].resize(65);
		}
	}
	std::vector < std::vector< float > > land;
	std::vector < std::vector< Normal > > normals;
};

//////////////////////////////////////////////////////////////////////////
//nif output stuff

/**
* @brief get material property to be added to the mesh
*/
Niflib::NiPropertyRef getMatProp(){
	using namespace Niflib;

	NiMaterialPropertyRef matPropRef = new NiMaterialProperty;
	matPropRef->SetName("01 - Default");

	//set base nif colours
	Color3 colour;
	colour.Set(0.588, 0.588, 0.588);
	matPropRef->SetAmbientColor(colour);
	matPropRef->SetDiffuseColor(colour);

	colour.Set(0.9, 0.9, 0.9);
	matPropRef->SetSpecularColor(colour);

	colour.Set(0, 0, 0);
	matPropRef->SetEmissiveColor(colour);

	return DynamicCast<NiProperty>(  matPropRef );
}

/**
* @brief gets the texutre. It is a random texture, so you would have to set in to what you want in nifskope
*/
Niflib::NiPropertyRef getTexture(){
	using namespace Niflib;

	NiSourceTextureRef srcTx = new NiSourceTexture;
	srcTx->SetExternalTexture("MyTexture.dds");
	NiTexturingPropertyRef txPropRef = new NiTexturingProperty;

	TexDesc txD;
	txD.source = srcTx;
	txPropRef->SetTexture(0, txD);
	return DynamicCast<NiProperty>(  txPropRef ); //now ready to be added to the mesh.
}


void addToNif(Land l, long offx, long offy, CellMeshList& cml){
	using namespace Niflib;

	std::vector<Vector3> verts;
	std::vector<Vector3> normals;
	std::vector<TexCoord> txCoords;
	std::vector<Triangle> tris;


	for ( int x = 0; x < 65; x++){
		for ( int y = 0; y < 65; y++){
			verts.push_back(
				Vector3(x*128,y*128,l.land[x][y]*8)
			);

			//vector needs to be normalized
			float nx = l.normals[x][y].x;
			float ny = l.normals[x][y].y;
			float nz = l.normals[x][y].z;
			float mag = sqrt(nx*nx+ny*ny+nz*nz);
			nx/=mag;
			ny/=mag;
			nz/=mag;

			normals.push_back(
				Vector3(nx,ny,nz)
			);

			txCoords.push_back(
				TexCoord(x/(float)65, y/(float)65)
			);
		}
	}

	//ak. Hacky
	const unsigned vertNum = 65 * 65;
	const unsigned meshLength = sqrt((float) vertNum);

	const unsigned loopSize = (vertNum) - (meshLength * 2) + 65;
	for( unsigned i = 0; i < loopSize; i++){
		unsigned iTmp = i;
		while ( iTmp > (meshLength - 1) )
			iTmp -= meshLength;

		if ( iTmp == (meshLength - 1) )
			continue;

		Triangle tri;

		tri.v1 = i;
		tri.v2 = i + meshLength;
		tri.v3 = i + 1;
		tris.push_back(tri);


		tri.v1 = i + meshLength;
		tri.v2 = i + meshLength + 1;
		tri.v3 = i + 1;
		tris.push_back(tri);
	}

	NiTriShapeRef shape = new NiTriShape;
	shape->SetName("TxtMesh");
	shape->AddProperty(getMatProp());
	shape->AddProperty(getTexture());


	NiTriShapeDataRef triData = new NiTriShapeData;
	triData->SetVertices(verts);
	triData->SetNormals(normals);
	triData->SetTriangles(tris);

	//uv
	triData->SetUVSetCount(1);
	triData->SetUVSet(0, txCoords);

	//ad it to the TriShape
	NiGeometryDataRef gRef = DynamicCast<NiGeometryData>( triData );
	shape->SetData(gRef);


	NiAVObjectRef avObj = DynamicCast<NiAVObject>(  shape );

	//make it so the center of the cell is on 0, 0
	Vector3 vec3;
	vec3.x = offx;
	vec3.y = offy;
	avObj->SetLocalTranslation(vec3);

	cml.push_back(avObj);
}

/**
* @brief writes the given nif to the given file.
*/
void dumpNif(const std::string& n, CellMeshList& l){
	using namespace Niflib;
	NiNodeRef node = new NiNode;

	for ( std::list<NiAVObjectRef>::iterator iter = l.begin(); l.end() != iter; ++iter)
		if ( *iter )
			node->AddChild(*iter);


	NiObjectRef obj = DynamicCast<NiObject>( node );
	unsigned ver = 67108866;
	WriteNifTree(n, obj, NifInfo(ver));
}


//////////////////////////////////////////////////////////////////////////
//image output stuff

void addToRaw(std::vector < std::vector < float > > d, int cellX, int cellY){
	gHeightmap[cellX][cellY] = d;
}

void dumpRaw(long lowerX, long lowerY, long upperX, long upperY, MultiHeightMap& mhm){


	MultiHeightMap hm;

	//check all our cells exist
	for ( int x = lowerX; x <= upperX; ++x ){
		for ( int y = lowerY; y <= upperY; ++y ){
			bool found = false;
			MultiHeightMap::iterator iter1 = mhm.find(x);

			if ( iter1 != mhm.end() ){
				if ( iter1->second.find(y) != iter1->second.end() )
					found = true;
			}

			std::vector<float> v1; v1.resize(65, 0);
			if ( !found )
				mhm[x][y].resize(65, v1);


			std::vector<float> v2; v2.resize(65, 0);
			hm[x][y].resize(65, v2);

			for ( int j = 0; j < 64; ++j){
				for ( int k = 0; k < 64; ++k){
					hm[x][y][j][k] = (mhm[x][y][j][k] +  mhm[x][y][j+1][k] + mhm[x][y][j][k+1] + mhm[x][y][j+1][k+1])/4;
				}
			}

		}
	}

	#define SIZE_OF_MAP 64
	float fmax = -8192;
	float fmin = 8192;

	//get the max and min heights
	for ( MultiHeightMap::iterator iter1 = mhm.begin();	mhm.end() != iter1; ++iter1){
		for ( std::map < int, fHeightMap >::iterator iter2 = (*iter1).second.begin();
			iter2 != iter1->second.end(); ++iter2){

			for ( int x = 0; x < SIZE_OF_MAP; ++x ){
				for ( int y = 0; y < SIZE_OF_MAP; ++y ){
					fmax = max(fmax, iter2->second[x][y]);
					fmin = min(fmin, iter2->second[x][y]);
				}
			}			
		}
	}

	//precalc diff between high/low
	float diff = fmin - fmax;
	if ( diff < 0 ) diff*=-1;


	std::ofstream ofs("heights.dump.raw", std::ios::binary );


	for ( int y = 0; y < (upperY+1 - lowerY) * (SIZE_OF_MAP); ++y ){
		float floorY = floor(y/(float)SIZE_OF_MAP);
		for ( int x = 0; x < (upperX+1 - lowerX) * (SIZE_OF_MAP); ++x ){
			float floorX = floor(x/(float)SIZE_OF_MAP);

			float o1 = mhm	[lowerX + floorX][lowerY + floorY][(x-(floorX*SIZE_OF_MAP))][(y-(floorY*SIZE_OF_MAP))];
			char o = o1/(float)(diff)*127+128;
			ofs.write((char*)&o, 1);
		}
	}
	ofs.close();

}

//////////////////////////////////////////////////////////////////////////
//esp/m functions

/**
* @brief gets the land data from the land record
*/
Land readLandData(std::ifstream& ifs, long rend){
	Land l;

	long subRecSize;
	int count = 0;
	while ( ifs.tellg() < rend ){
		char dataType[5];
		ifs.read(dataType, 4);
		dataType[4] = '\0';
		//cell data

		ifs.read ((char *)&subRecSize, sizeof(long));
		long subRecEnd = subRecSize + ifs.tellg();

		if ( strcmp(dataType, "VNML") == 0 ){
			for ( int x = 0; x < 65; x++ ){
				for ( int y = 0; y < 65; y++){
					ifs.read((char*)&l.normals[x][y], sizeof(Normal));
				}
			}
		}

		 if ( strcmp(dataType, "VHGT") == 0){

			float offset;
			ifs.read ((char *)&offset, sizeof(float));

			for(int y = 0; y < 65; y++) {
				char x;
				ifs.get(x);	
				offset += x;
				l.land[0][y] = offset;

				float pos = offset;

				for(int x = 1; x< 65; x++) {

					char c;
					int tmp = 0;
					ifs.get(c);						
					pos += c;

					l.land[x][y] = pos;
				}
			}
		 }
		 ifs.seekg(subRecEnd);
	}
	return l;
}

/**
* @brief reads the given data file, and extracts data
* @param lowerX, lowerY the lower bounds of the area to get
* @param upperX, upperY the uppder bounds of the area to get
*/
bool readDataFile(const std::string& f, long lowerX, long lowerY, long upperX, long upperY, CellMeshList& cml, MultiHeightMap& mhm){
	std::ifstream ifs(f.c_str(), std::ios::binary | std::ios::in);

	if ( !ifs.is_open() ){
		std::exception("Cannot open file");
		return 0;
	}

	long fileSize;

	ifs.seekg (0, std::ios::end);
	fileSize = ifs.tellg();
	ifs.seekg (0, std::ios::beg);

	//skip the hdr
	ifs.seekg((long)ifs.tellg() + 4);
	long hdrSize;
	ifs.read ((char *)&hdrSize, sizeof(int));
	ifs.seekg((long)ifs.tellg() + 8);
	ifs.seekg(hdrSize + 16);

	while ( ifs.tellg() < fileSize  && (long) ifs.tellg() != -1  ){
		char recType[5]; //type
		ifs.get(recType, 5);

		long recordSize; //record size
		ifs.read ((char *)&recordSize, 4);


		//scip hdr / flags
		ifs.seekg((long)ifs.tellg() + 8);

		long recordEndPos = recordSize + ifs.tellg();

		if ( strcmp("LAND", recType) == 0 ){
			LandSquare lsq;

			char dataType[5];
			ifs.read(dataType, 4);
			dataType[4]='\0';
			if ( strcmp("INTV", dataType) == 0 ){
				ifs.seekg((long)ifs.tellg() + 4);
				ifs.read((char*)&lsq, sizeof(LandSquare));

				if ( lowerX <= lsq.x  && lowerY <= lsq.y && upperX >= lsq.x && upperY >= lsq.y){
					Land l = readLandData(ifs, recordEndPos);

					long offx = -4096 + ((lsq.x - lowerX) * 8192);
					long offy = -4096 + ((lsq.y - lowerY) * 8192);

					mhm[lsq.x][lsq.y] = l.land;
					addToNif(l, offx, offy,cml);
				}
			}

		}


		ifs.seekg(recordEndPos);
	}

	return true;

}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]){

	if ( argc < 4 ){
		std::cout << "Usage:\n\"data file\" lowerx lowery [upperx] [uppery]";
		return 1;
	}

	std::string input(argv[1]);
	long highX, highY;
	long lowX = fromString<long>(std::string(argv[2]));
	long lowY = fromString<long>(std::string(argv[3]));
	if ( argc > 4 ){
		highX = fromString<long>(std::string(argv[4]));
		highY = fromString<long>(std::string(argv[5]));
	}else{
		highX = lowX;
		highY = lowY;
	}


	MultiHeightMap hm;
	CellMeshList meshList;

	try{
		readDataFile(input, lowX,lowY, highX, highY, meshList, hm);
		dumpNif("default.nif", meshList);
		dumpRaw(lowX, lowY, highX, highY, hm);
	}catch(const std::exception& e){
		std::cout << "An Error Occurred\n" << e.what();
	}


}