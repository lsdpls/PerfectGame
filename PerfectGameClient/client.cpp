/*
	Simple udp client
*/
#define NOMINMAX

#include <iostream>
#include <memory>
#include <chrono>
#include "UdpSocket.h"
#include "../GameState/GameState.h"
#include <SFML/Graphics.hpp>


//ip address of udp server
std::string const kIpAddr = "127.0.0.1";
//The port on which to listen for incoming data
u_short const kPort = 8888;
size_t const kBufferSize = 512;
char buffer[kBufferSize];
bool firstEnter = 1;


void sleep(unsigned long us)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	while (microseconds.count() < us)
	{
		finish = std::chrono::high_resolution_clock::now();
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	}
}

int main(int argc, const char* argv[])
{
	std::unique_ptr<UdpSocket> sock_ptr;
	std::string name = "Name";
	std::cout << "Enter name:\n";
	std::cin >> name;
	GameState state;

	try
	{
		sock_ptr = std::make_unique<UdpSocket>(kIpAddr, kPort);
	}
	catch (std::exception const& err)
	{
		std::cout << "Couldn't init socket: " << err.what() << "\n";
		exit(EXIT_FAILURE);
	}

	//start communication
	//send the message
	if (sock_ptr->send(name.c_str(), name.length()) != 0)
	{
		std::cout << "Failed to send\n";
		exit(EXIT_FAILURE);
	}

	std::cout << "request sent\n";

	//
	bool blockChanged = 0;// изменён ли блок
	PlayerPos blockPos;//позиция изменённого блока

	int i = 0;
	sf::RenderWindow window(sf::VideoMode(800, 600), "Perfect Game");

	sf::Texture GroundT;
	GroundT.loadFromFile("ground.png");

	sf::Texture AirT;
	AirT.loadFromFile("air.png");

	sf::Texture chelRT;
	chelRT.loadFromFile("chel_r.png");

	sf::Texture chelLT;
	chelLT.loadFromFile("chel_l.png");

	sf::Sprite PlayerS;
	sf::Sprite BlockS;
	sf::Sprite FriendS;
	FriendS.setTexture(chelRT);


	sf::Font font;
	font.loadFromFile("caviar-dreams.ttf");

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(12);
	text.setFillColor(sf::Color::Black);
	text.setStyle(sf::Text::Bold);
	
	GameState::Block block = GameState::Block::Ground;
	bool isLeft = 0;
	PlayerPos client_pos;
	//

	while (1)
	{
		//receive a reply and print it
		size_t sz = kBufferSize;
		if (sock_ptr->recv(buffer, sz) != 0)
		{
			std::cout << "No data to recv\n";
			sleep(1e5);
			continue;
			//exit(EXIT_FAILURE);
		}

		//std::cout << "Received game state: " << sz << "\n";
		state.deserialize(buffer, sz);
		Player* p = state.getPlayer(name);
		PlayerPos curr_pos = p->getPos();
		
		
		if (firstEnter)
		{
			client_pos = curr_pos;
			firstEnter = 0;
		}

		PlayerS.setPosition(sf::Vector2f(50.f * client_pos.first, 50.f * client_pos.second));
		if (isLeft)
			PlayerS.setTexture(chelLT);
		else 
			PlayerS.setTexture(chelRT);
		

		text.setPosition(50.f * client_pos.first, 50.f * client_pos.second);
		text.setString(name);
		
		
		
		for (int i = 0; i < state.getRows(); i++)
		{
			for (int j = 0; j < state.getCols(); j++)
			{
				if (state.getMap()[i][j] == GameState::Block::Background)
					BlockS.setTexture(AirT);
				else if (state.getMap()[i][j] == GameState::Block::Ground)
					BlockS.setTexture(GroundT);

				BlockS.setPosition(sf::Vector2f(j * 50.f, i * 50.f));

				window.draw(BlockS);
			}
		}
		

		for (auto &it : state.getPlayers())
		{
			if (it.first != name)
			{
				FriendS.setPosition(it.second.getPos().first * 50.f, it.second.getPos().second * 50.f);
				text.setPosition(it.second.getPos().first * 50.f, it.second.getPos().second * 50.f);
				text.setString(it.first);
				window.draw(FriendS);
				
				window.draw(text);
			}
		}
		
		
		
		
		
		
		sf::Event event;

		window.pollEvent(event);

		if (event.type == sf::Event::Closed)
		{
			window.close();
			std::cout << "Exit\n";
			break;
		}
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::D)
			{
				if (client_pos.first + 1 == 16)
					continue;
				if (state.getMap()[client_pos.second][client_pos.first + 1] == GameState::Block::Background)
				{
					client_pos.first++;
					client_pos.first = client_pos.first % state.getCols();
					isLeft = 0;
					if (isLeft)
						PlayerS.setTexture(chelLT);
					else
						PlayerS.setTexture(chelRT);
				}
			}
			if (event.key.code == sf::Keyboard::A)
			{
				if (client_pos.first - 1 == -1)
					continue;
				if (state.getMap()[client_pos.second][client_pos.first - 1] == GameState::Block::Background)
				{
					client_pos.first--;
					client_pos.first = client_pos.first % state.getCols();
					isLeft = 1;
					if (isLeft)
						PlayerS.setTexture(chelLT);
					else
						PlayerS.setTexture(chelRT);
				}
				

			}
			if (event.key.code == sf::Keyboard::W)
			{
				if (client_pos.second - 1 == -1)
					continue;
				if (state.getMap()[client_pos.second - 1][client_pos.first] == GameState::Block::Background)
				{
					client_pos.second--;
					client_pos.second = client_pos.second % state.getRows();
					if (isLeft)
						PlayerS.setTexture(chelLT);
					else
						PlayerS.setTexture(chelRT);
				}
				

			}
			if (event.key.code == sf::Keyboard::S)
			{
				if (client_pos.second + 1 == 12)
					continue;
				if (state.getMap()[client_pos.second + 1][client_pos.first] == GameState::Block::Background)
				{
					client_pos.second++;
					client_pos.second = client_pos.second % state.getRows();
					if (isLeft)
						PlayerS.setTexture(chelLT);
					else
						PlayerS.setTexture(chelRT);
				}
				

			}
			if (event.key.code == sf::Keyboard::E)
			{
				if (isLeft)
				{
					if (client_pos.first - 1 == -1)
						continue;
					if (state.getMap()[client_pos.second][client_pos.first - 1] == GameState::Block::Background)
					{
						blockChanged = 1;
						blockPos.first = client_pos.first - 1;
						blockPos.second = client_pos.second;
						block = GameState::Block::Ground;
					}
					else if (state.getMap()[client_pos.second][client_pos.first - 1] == GameState::Block::Ground)
					{
						blockChanged = 1;
						blockPos.first = client_pos.first - 1;
						blockPos.second = client_pos.second;
						block = GameState::Block::Background;
					}
				}
				else
				{
					if (client_pos.first + 1 == 16)
						continue;
					if (state.getMap()[client_pos.second][client_pos.first + 1] == GameState::Block::Background)
					{
						blockChanged = 1;
						blockPos.first = client_pos.first + 1;
						blockPos.second = client_pos.second;
						block = GameState::Block::Ground;
					}
					else if (state.getMap()[client_pos.second][client_pos.first + 1] == GameState::Block::Ground)
					{
						blockChanged = 1;
						blockPos.first = client_pos.first + 1;
						blockPos.second = client_pos.second;
						block = GameState::Block::Background;
					}
				}



				

			}
			if (event.key.code == sf::Keyboard::Q)
			{
				if (client_pos.second + 1 == 12)
					continue;
				if (state.getMap()[client_pos.second+1][client_pos.first] == GameState::Block::Background)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first;
					blockPos.second = client_pos.second + 1;
					block = GameState::Block::Ground;
				}
				else if (state.getMap()[client_pos.second + 1][client_pos.first ] == GameState::Block::Ground)
				{
					blockChanged = 1;
					blockPos.first = client_pos.first;
					blockPos.second = client_pos.second + 1;
					block = GameState::Block::Background;
				}
			}
		}
		




		text.setPosition(50.f * client_pos.first, 50.f * client_pos.second);
		text.setString(name);
		window.draw(PlayerS);
		window.draw(text);


		window.display();

		
		p->updatePos(client_pos.first, client_pos.second);
		sz = kBufferSize;

		if (blockChanged)
		{
			p->serialize(buffer, sz, blockPos.second , blockPos.first, (char)block);
			blockChanged = 0;
		}
		else
			p->serialize(buffer, sz);

		
		
		

		if (sock_ptr->send(buffer, sz) != 0)
		{
			std::cout << "Failed to send pos\n";
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}