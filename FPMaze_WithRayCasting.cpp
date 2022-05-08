
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>

#include <stdio.h>
#include <Windows.h>//has to be the last included

const int SCREENWIDTH = 120;//this is to protect my screen size
const int SCREENHEIGHT = 40; //and since they cant be negative

int mapHeight = 16;
int mapWidth = 16;

float PlayerX = 8.0f;//Player's coordinates
float PlayerY = 8.0f;//Player's coordinates

float PlayerAngle = 0.0f;
float FieldOfView = 3.14159f / 4.0f;

float Depth = 16.0f;
float speed = 5.0f;

int main()
{
	wchar_t* screen = new wchar_t[SCREENWIDTH * SCREENHEIGHT];//here is our twodimantional array of wchar

	//Here making a buffer and indicating the deired access, setting security attributes, shareMode, flags and buffer data
	HANDLE Console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	//indicating to the buffer that it's going to be our console
	SetConsoleActiveScreenBuffer(Console);

	DWORD BytesWritten = 0;//Needed for debugging

	std::wstring map;
	map += L"##########.....#"; //doig the map line by line;
	map += L"#....#.........#";
	map += L"#....######....#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#......####....#";
	map += L"#..............#";
	map += L"###########....#";
	map += L"#..............#";
	map += L"#.....###......#";
	map += L"#.....###......#";
	map += L"#..............#";
	map += L"#.........######";
	map += L"################";

	//Declare two time variables and initialize them with the current time
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	
	while (1) {

		//reset one of the variables to a new current time;
		tp2 = std::chrono::system_clock::now();

		//declare a class duration that stores the delta between two times
		std::chrono::duration<float> elapsedTime = tp2 - tp1;

		//assign the later time to the first time varriable
		tp1 = tp2;

		//Returns the time duration
		float ElapsedTime = elapsedTime.count();

		//Controlls system
		     //Detects if the key was pressed after the last query 
		     //it also said to ignore all the bits accept the most signifcant one with bitwise AND + 0x8000
		     //which in binary will look as 10000000 00000000 00000000 00000000 with the most significant bit being 1
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
		{
			PlayerAngle -= (speed * 0.5f)*ElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
		{
			PlayerAngle += (speed * 0.5f)* ElapsedTime;
		}


		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			//using the unit vector and scaling it to create movement 
			PlayerX += sinf(PlayerAngle) * speed * ElapsedTime;;
			PlayerY += cosf(PlayerAngle) * speed * ElapsedTime;;

			//handling the collision by substracting the same speed that we're adding to create the stopping affect
			if (map.c_str()[(int)PlayerX * mapWidth + (int)PlayerY] == '#')
			{
				PlayerX -= sinf(PlayerAngle) * speed * ElapsedTime;;
				PlayerY -= cosf(PlayerAngle) * speed * ElapsedTime;;
			}
		}

		
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			PlayerX -= sinf(PlayerAngle) * speed * ElapsedTime;;
			PlayerY -= cosf(PlayerAngle) * speed * ElapsedTime;;

			if (map.c_str()[(int)PlayerX * mapWidth + (int)PlayerY] == '#')
			{
				PlayerX += sinf(PlayerAngle) * speed * ElapsedTime;;
				PlayerY += cosf(PlayerAngle) * speed * ElapsedTime;;
			}
		}

		for (int x = 0; x < SCREENWIDTH; x++)
		{
			//RayAngle is going to go from the starting point which is -FOV/2 (in relation to theplayers position
			//to +FOV/2 and the second part of the function breaks it up int 120 steps (the width of the screen)
			float RayAngle = (PlayerAngle - FieldOfView/2.0f) + ((float)x / (float)SCREENWIDTH) * FieldOfView;
			
			float stepSize = 0.1f;
			float DistanceToWall = 0.0f;
			bool bHitTheWall = false;
			bool bHitBoundry = false;

			float EyeX = sinf(RayAngle); //Unit vector for the ray in player space
			float EyeY = cosf(RayAngle);

			while (!bHitTheWall && DistanceToWall< Depth) {

				DistanceToWall += stepSize;

				//These are integers because we only need the boudries (surfaces) everyting inbetween the surfaces is a wall
				int TestX = (int)(PlayerX + EyeX * DistanceToWall);
				int TestY = (int)(PlayerY + EyeY * DistanceToWall);

				//Is the ray out of bounds? If so - we hit the wall
				if (TestX < 0 || TestX >= mapWidth || TestY < 0 || TestY >= mapHeight)
				{
					bHitTheWall = true;
					DistanceToWall = Depth; //we're at the maximum distance as well
				}

				else { 

					if (map[TestX * mapWidth + TestY ]== '#')
					{
						bHitTheWall = true;

						// We cast a ray from the perfect corner of the tile to the player
						// We compare this ray to the rendring rays that we use to draw space
						// The more two of those rays are similar, the closer we are to the boundary
					
						//create the vector to store our ray cast from a perfect corner
						std::vector<std::pair<float, float>>p;

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++)
							{
								
								float vy = (float)TestY + ty - PlayerY; // Angle of corner to eye
								float vx = (float)TestX + tx - PlayerX;
								float d = sqrt(vx * vx + vy * vy); //distance from the corner
								float dot = (EyeX * vx / d) + (EyeY * vy / d);
								p.push_back(std::make_pair(d, dot));
							}
						}
						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });

						// First two/three are closest (we will never see all four)
						float fBound = 0.001;
						if (acos(p.at(0).second) < fBound) bHitBoundry = true;
						if (acos(p.at(1).second) < fBound) bHitBoundry = true;
					}
				}
			}

			//Calculating the distance floor and the cieling
			//For the cieling we find the center of the screen and substarct the screen heght in proportion to the distance from the wall
			//The floor simply mirrors the cieling;
			int Cieling = (float)(SCREENHEIGHT / 2.0) - SCREENHEIGHT/((float)DistanceToWall);
			int Floor = SCREENHEIGHT - Cieling;

			short Shade = ' ';

			//here we compare the distance between the player with a certain percentage of depth
			//so if the distance is less then 25% of depth - we're close and the shade of the wall is lighter
			//the further away we are - the darker it gets
			if (DistanceToWall <= Depth / 4.0f)		   Shade = 0x2588; // here are the hex codes of the unicode shade charachters		
			else if (DistanceToWall < Depth / 3.0f)    Shade = 0x2593; 
			else if (DistanceToWall < Depth / 2.0f)	   Shade = 0x2592;
			else if (DistanceToWall < Depth)		   Shade = 0x2591;
			else									   Shade = ' ';		// not visible

			if (bHitBoundry)		Shade = ' '; 

			//drawing the columns to make the cieling, floor and walls appear.
			for (int y = 0; y < SCREENHEIGHT; y++)
			{
				if (y <= Cieling) 
					screen[y * SCREENWIDTH + x] = ' ';
				else if (y > Cieling && y <= Floor)
					screen[y * SCREENWIDTH + x] = Shade;
				else {

					//measures the distance from y to the end of the screen and assigns shaders
					float b = 1.0f - (((float)y - SCREENHEIGHT/2.0f) / ((float)SCREENHEIGHT/2.0f));
					if (b < 0.25)		Shade = 'x';
					else if (b < 0.5)	Shade = '*';
					else if (b < 0.75)	Shade = '.';
					else if (b < 0.9)	Shade = '-';
					else				Shade = ' ';

					screen[y * SCREENWIDTH + x] = Shade;
				}
			}

		}

		//Drawing a 
		for (int nx = 0; nx < mapWidth; nx++)
			for (int ny = 0; ny < mapWidth; ny++)
			{
				screen[(ny + 1) * SCREENWIDTH + nx] = map[ny * mapWidth + nx];
			}

		screen[((int)PlayerX + 1) * SCREENWIDTH + (int)PlayerY] = 'P';

		screen[SCREENWIDTH * SCREENHEIGHT-1] = '\0';  // End of the line symbol to show the console when to stop drawing
		// {0,0} is needed to always start writing at the upper left corner
		WriteConsoleOutputCharacter(Console, screen, SCREENWIDTH * SCREENHEIGHT, { 0, 0 }, &BytesWritten);
	}

	
	return 0;

}

