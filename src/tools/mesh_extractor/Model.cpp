/*
 * Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model.h"
#include "MPQManager.h"
#include "Utils.h"

Model::Model( std::string path ) : IsCollidable(false), IsBad(false)
{
    Stream = MPQHandler->GetFile(Utils::FixModelPath(path));
    if (!Stream)
    {
        IsBad = true;
        return;
    }
    Header.Read(Stream);
    if (Header.OffsetBoundingNormals > 0 && Header.OffsetBoundingVertices > 0 &&
        Header.OffsetBoundingTriangles > 0 && Header.BoundingRadius > 0.0f)
    {
        IsCollidable = true;
        ReadVertices(Stream);
        ReadBoundingNormals(Stream);
        ReadBoundingTriangles(Stream);
    }
}

Model::~Model()
{
    if (Stream)
        fclose(Stream);
}

void Model::ReadVertices( FILE* stream )
{
    fseek(stream, Header.OffsetBoundingVertices, SEEK_SET);
    Vertices.reserve(Header.CountBoundingVertices);
    for (uint32 i = 0; i < Header.CountBoundingVertices; ++i)
    {
        Vertices.push_back(Vector3::Read(stream));
        if (Constants::ToWoWCoords)
            Vertices[i] = Utils::ToWoWCoords(Vertices[i]);
    }
}

void Model::ReadBoundingTriangles( FILE* stream )
{
    fseek(stream, Header.OffsetBoundingTriangles, SEEK_SET);
    Triangles.reserve(Header.CountBoundingTriangles / 3);
    for (uint32 i = 0; i < Header.CountBoundingTriangles / 3; i++)
    {
        Triangle<uint16> tri;
        tri.Type = Constants::TRIANGLE_TYPE_DOODAD;
        int count = 0;
        count += fread(&tri.V0, sizeof(uint16), 1, stream);
        count += fread(&tri.V1, sizeof(uint16), 1, stream);
        count += fread(&tri.V2, sizeof(uint16), 1, stream);
        if (count != 3)
            printf("Model::ReadBoundingTriangles: Error reading data, expected 3, read %d\n", count);
        Triangles.push_back(tri);
    }
}

void Model::ReadBoundingNormals( FILE* stream )
{
    fseek(stream, Header.OffsetBoundingNormals, SEEK_SET);
    Normals.reserve(Header.CountBoundingNormals);
    for (uint32 i = 0; i < Header.CountBoundingNormals; i++)
        Normals.push_back(Vector3::Read(stream));
}

