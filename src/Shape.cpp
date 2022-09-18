
#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

vec3 reduceFloatLength(vec3 vec){
	while (vec.x > 1e+8 || vec.y > 1e+8 || vec.z > 1e+8 ||
       vec.x < -1e+8 || vec.y < -1e+8 || vec.z < -1e+8){
		vec.x /= 2.0;
		vec.y /= 2.0;
		vec.z /= 2.0;
	}
	return vec;
}

vec3 replaceNan(vec3 normal){
	vec3 newNormal;
	if(isnan(normal.x)){
		newNormal.x = 0;
	}
	if(isnan(normal.y)){
		newNormal.y = 0;
	}
	if(isnan(normal.z)){
		newNormal.z = 0;
	}
	return newNormal;
}

std::vector<float> normalizeBuf(vector<float> noNorBuf, int size){
	std::vector<float> newNorBuf(size);
	vec3 normal, totalNormals;
	for(int k = 0; k < size - 2; k++){
		totalNormals.x = noNorBuf[k];
		totalNormals.y = noNorBuf[k + 1];
		totalNormals.z = noNorBuf[k + 2];
		//totalNormals = reduceFloatLength(totalNormals);

		normal = normalize(totalNormals);
		normal = reduceFloatLength(normal);

		

		normal = replaceNan(normal);


		
		newNorBuf[k] = normal.x;
		newNorBuf[k + 1] = normal.y;
		newNorBuf[k + 2] = normal.z;

		//cout << "normal: " << normal.x << " " << normal.y << " " << normal.z<< "\n";
		k++;
		k++;
	}
		return newNorBuf;
}



void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));
	int size = posBuf.size();
	std::vector<float> noNorBuf(size);
	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		
		
		
		//compute normal
		//make var norBuf like vertBuf
		//for every face
			//get 3 verts like P1
			//compute 2 edges
				//compute normal using cross product
				//add face normal to its vertecies normal
	
		//compute overall normal at end

		vec3 vert1, vert2, vert3;
		vec3 edge1, edge2;
		
		//vector noNorBuf[size];

		//cout << "check\n" << posBuf.size() << "   " << noNorBuf.size() << "\n";

		vec3 faceNormal;

		//populate noNorBuf
		for(int v = 0; v < posBuf.size(); v++){
			noNorBuf[v] = 0.0;
			//cout << v;
		}
		

		for(int i = 0; i < eleBuf.size(); i += 3){	
			int index1 = eleBuf[i];
			int index2 = eleBuf[i + 1];
			int index3 = eleBuf[i + 2];

			vert1.x = posBuf[(index1 * 3)];
			vert1.y = posBuf[(index1 * 3 + 1)];
			vert1.z = posBuf[(index1 * 3 + 2)];

			vert2.x = posBuf[(index2 * 3)];
			vert2.y = posBuf[(index2 * 3 + 1)];
			vert2.z = posBuf[(index2 * 3 + 2)];

			vert3.x = posBuf[(index3 * 3)];
			vert3.y = posBuf[(index3 * 3 + 1)];
			vert3.z = posBuf[(index3 * 3 + 2)];
			
			edge1.x = vert2.x - vert1.x;
			edge1.y = vert2.y - vert1.y;
			edge1.z = vert2.z - vert1.z;

			edge2.x = vert3.x - vert1.x;
			edge2.y = vert3.y - vert1.y;
			edge2.z = vert3.z - vert1.z;
			

			//cross product
			faceNormal = cross(edge1, edge2);
			//faceNormal = replaceNan(faceNormal);

			//reduce float length and normalize
			faceNormal = reduceFloatLength(faceNormal);
			faceNormal = normalize(faceNormal);
			//faceNormal = replaceNan(faceNormal);

			faceNormal = reduceFloatLength(faceNormal);

			//add normals to coorsponding verts
			noNorBuf[(index1 * 3)] += faceNormal.x;
			noNorBuf[(index1 * 3 + 1)] += faceNormal.y;
			noNorBuf[(index1 * 3 + 2)] += faceNormal.z;

			noNorBuf[(index2 * 3)] += faceNormal.x;
			noNorBuf[(index2 * 3 + 1)] += faceNormal.y;
			noNorBuf[(index2 * 3 + 2)] += faceNormal.z;

			noNorBuf[(index3 * 3)] += faceNormal.x;
			noNorBuf[(index3 * 3 + 1)] += faceNormal.y;
			noNorBuf[(index3 * 3 + 2)] += faceNormal.z;

			
		}
		norBuf = normalizeBuf(noNorBuf, size);
		
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));


		//norBufID = 1;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
