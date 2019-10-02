#include "client.h"

int main()
{
	char inp;
	int res;

	// Load socket lib
	WSADATA wsaData;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NO_ERROR)
	{
		std::cout << "Failed to load socket! err: " << res;
		std::cin >> inp;
		WSACleanup();
		return 0;
	}

	// Init socket
	SOCKET conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
	{
		std::cout << "Failed to init socket! err: " << WSAGetLastError();
		std::cin >> inp;
		WSACleanup();
		return 0;
	}

	// Define host addr
	sockaddr_in hostAddr = sockaddr_in();
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = inet_addr("192.168.0.13");
	hostAddr.sin_port = htons(1337);

	// Connect
	res = connect(conn, (SOCKADDR*)&hostAddr, sizeof(hostAddr));
	if (res == SOCKET_ERROR)
	{
		std::cout << "Failed to connect socket!";
		std::cin >> inp;
		WSACleanup();
		return 0;
	}

	// Init controller inp
	XINPUT_STATE state;
	memset(&state, 0, sizeof(XINPUT_STATE));

	// App loop
	short angle = 0, speed = 0, newAngle = 0, newSpeed = 0;
	while (true)
	{
		// Controller data inp
		if (XInputGetState(0, &state) == ERROR_SUCCESS)
		{
			newAngle = newSpeed = 0;

			// Apply threshold on input
			if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				newSpeed -= state.Gamepad.bLeftTrigger;
			if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				newSpeed += state.Gamepad.bRightTrigger;
			if (state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				newAngle = (short)((float)state.Gamepad.sThumbLX / SHRT_MAX * 256);

			// New angle
			if (angle != newAngle)
			{
				angle = newAngle;
				
				byte *buffer = new byte[3];
				buffer[0] = 1;
				buffer[1] = angle & 0xFF;
				buffer[2] = (angle >> 8) & 0xFF;

				res = send(conn, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
				if (res == SOCKET_ERROR)
				{
					std::cout << "Failed to send data! err: " << WSAGetLastError();
					std::cin >> inp;
					WSACleanup();
					return 0;
				}

				std::cout << "Angle: " << angle << "\n";

				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			// New speed
			if (speed != newSpeed)
			{
				speed = newSpeed;
				
				byte *buffer = new byte[3];
				buffer[0] = 2;
				buffer[1] = speed & 0xFF;
				buffer[2] = (speed >> 8) & 0xFF;

				char *data = reinterpret_cast<char*>(buffer);

				res = send(conn, data, sizeof(buffer), 0);
				if (res == SOCKET_ERROR)
				{
					std::cout << "Failed to send data! err: " << WSAGetLastError();
					std::cin >> inp;
					WSACleanup();
					return 0;
				}

				std::cout << "Speed: " << speed << " : " << data << "\n";

				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		else
		{
			std::cout << "Failed to connect to controller\n";

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
	}

	WSACleanup();
	return 0;
}