#include <iostream>
#include <string>
#include <sstream>
using std::cout;
using std::endl;
using std::string;
using std::stringstream;

#include "AlefLoginGlobal.h"
#include "AlefLoginClientLogin.h"
#include "AlefMD5Crypto.h"

AlefLoginClientLogin::AlefLoginClientLogin()
{

}

AlefLoginClientLogin::~AlefLoginClientLogin()
{

}

bool AlefLoginClientLogin::processPacket(AlefSocket& sock, AlefPacket * packet)
{
	/*Alef::INT8, Alef::CHAR, Alef::CHAR, Alef::INT8, Alef::CHAR, Alef::INT8, Alef::INT32, Alef::CHAR, Alef::PACKET, Alef::PACKET, Alef::INT32, Alef::PACKET, Alef::CHAR, Alef::CHAR, Alef::INT32, Alef::INT32*/
	/* 1, 32, 49, 1, 33, 1, 1, 32, 1, 1, 1, 1, 2049, 5, 1, 1*/
	Int8 i8Operation = 0;
	unsigned char acct[49] = { 0 };
	UInt8 acctLen = 0, pwLen = 0;
	unsigned char pw[33] = { 0 };
	Int8 i8Var = 0;
	char char1Var[33] = { 0 };
	//char 

	pktInterface->processPacket(packet, &i8Operation, 0, acct, &acctLen, pw, &pwLen, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	switch(i8Operation)
	{
		case 0: //Initial Packet Opcode
			return processInitialLoginPacket(sock, packet); break;
		case 1: //User Login Packet Opcode
			return processUserLoginPacket(sock, packet, acct, acctLen, pw, pwLen); break;
		case 3: //User World Union Info
			return processUnionInfo(sock, packet); break;
		case 6: //User Character List Request
			return processCharacterList(sock, packet); break;
		case 0x05: //Character Creation Request 0x0D 0x05
			return processCharacterCreation(sock, packet); break;
		case 8: //World Enter 0x0D 0xC5
			return processWorldEnterRequest(sock, packet); break;
		default:
		{
			stringstream errorMsg;
			errorMsg << "User Connect processPacket Unhandled Operation: " << (int)i8Operation;
			LOG(errorMsg.str(), FATAL);
			return false;
		}
	}
	return false;
}

bool AlefLoginClientLogin::processInitialLoginPacket(AlefSocket& sock, AlefPacket * packet)
{
	LOG("processInitialLoginPacket");

	Int8 i8Operation = 0, i8Unk = 0;
	unsigned char hashKey[] = "12345678";

	SharedPtr<AlefPacket> response = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, hashKey, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); //MD5 Crypto Packet

	sock.sendPacket(response);

	return true;
}

bool AlefLoginClientLogin::processUserLoginPacket(AlefSocket& sock, AlefPacket * packet, unsigned char* acct, UInt8 acctLen, unsigned char* pw, UInt8 pwLen)
{
	LOG("processUserLoginPacket");
	AlefMD5Crypto md5Crypt;

	md5Crypt.decryptString(acct, acctLen);
	md5Crypt.decryptString(pw, pwLen);

	stringstream acctMsg;
	acctMsg << "Account: " << acct << " " << "Password: " << pw;
	LOG(acctMsg.str());

	Int8 i8Operation = 1;
	Int32 i32Unk = 0;

	SharedPtr<AlefPacket> signOnResponse = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, acct, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &i32Unk, &i32Unk);

	sock.sendPacket(signOnResponse);

	return true;
}

bool AlefLoginClientLogin::processUnionInfo(AlefSocket& sock, AlefPacket * packet)
{
	int unionType = 0;
	//Alef::INT32, Alef::CHAR, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::CHAR
	SharedPtr<AlefPacket> miniCharInfo = pktInterface->buildMiniPacket(Alef::AGPMLOGIN_CHAR_INFO, 0, 0, 0, 0, &unionType, 0, 0, 0, 0);

	/*Alef::INT8, Alef::CHAR, Alef::CHAR, Alef::INT8, Alef::CHAR, Alef::INT8, Alef::INT32, Alef::CHAR, Alef::PACKET, Alef::PACKET, Alef::INT32, Alef::PACKET, Alef::CHAR, Alef::CHAR, Alef::INT32, Alef::INT32*/
	Int8 i8Operation = 3;
	SharedPtr<AlefPacket> unionResponse = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, 0, 0, 0, 0, 0, 0, &miniCharInfo, 0, 0, 0, 0, 0, 0, 0);

	sock.sendPacket(unionResponse);

	unsigned char name[] = "test";
	int maxChars = 1, index = 0;

	SharedPtr<AlefPacket> charName = pktInterface->buildMiniPacket(Alef::AGPMLOGIN_CHAR_INFO, 0, name, &maxChars, &index, 0, 0, 0, 0, 0);

	i8Operation = 4;

	SharedPtr<AlefPacket> charNameResponse = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, 0, 0, 0, 0, 0, 0, charName, 0, 0, 0, 0, 0, 0, 0);

	sock.sendPacket(charNameResponse);

	i8Operation = 5;
	unsigned char acct[] = "acct";

	SharedPtr<AlefPacket> charNameFinish = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, acct, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	sock.sendPacket(charNameFinish);

	return true;

	/*UInt8 operation = packet->GetPacketFlag(FlagIndex::FLAG_IDX1);
	switch (operation)
	{
		case 0x03:
		{
			LOG("Initial User Connect");
			AlefPacket response(0x0D, 0x01, 0x01);

			response.WriteUInt8(0x03);
			response.WriteUInt8(0x06);
			response.WriteUInt8(0x00);
			response.WriteUInt8(0x10);
			response.WriteUInt8(0x00);
			response.WriteUInt8(0x01);
			response.WriteUInt8(0x00);
			response.WriteUInt8(0x00);
			response.WriteUInt8(0x00);
			response.ClosePacket();

			sock.sendPacket(&response);

			//Send Character List
			AlefPacket charListRes(0x0D, 0x01, 0x01);

			charListRes.WriteUInt8(0x04); //Char List Operation
			
			charListRes.WriteUInt16(59); //Size
			charListRes.WriteUInt16(14); //Flag
			charListRes.WriteByteArray("test");
			for (int i = 0; i < 44; i++)
				charListRes.WriteUInt8(0); //Spacing
			charListRes.WriteUInt8(4); //name Length
			charListRes.WriteUInt8(1); //Unk
			for (int i = 0; i < 7; i++)
				charListRes.WriteUInt8(0); //spacing
			
			charListRes.ClosePacket();
			
			sock.sendPacket(&charListRes);

			AlefPacket response2(0x0D, 0x05, 0x00);
			response2.WriteUInt8(0x05);
			response2.WriteByteArray("test");
			for (int i = 0; i < 45; i++)
				response2.WriteUInt8(0);
			response2.ClosePacket();

			sock.sendPacket(&response2);
		}break;
		default:
		{
			stringstream errorMsg;
			errorMsg << "User Connect Unhandled Operation: " << (int)operation;
			LOG(errorMsg.str(), WARNING);
			return false;
		} break;
	}*/
}

bool AlefLoginClientLogin::processCharacterList(AlefSocket& sock, AlefPacket * packet)
{
			/*AlefPacket charResponse(0x04, 0x11, 0x00);
			charResponse.WriteUInt8(0x40);
			charResponse.WriteUInt8(0);
			charResponse.WriteUInt8(0x13);
			charResponse.WriteUInt8(0x65);
			for (int i = 0; i < 7; i++)
				charResponse.WriteUInt8(0);
			charResponse.ClosePacket();
			sock.sendPacket(&charResponse);

			sendDummyCharacter(sock);

			AlefPacket charListComplete(0x02, 0x13, 0x00);
			charListComplete.WriteUInt16(0);
			charListComplete.WriteUInt8(0x01);
			charListComplete.WriteUInt8(0x65);
			charListComplete.WriteUInt16(0);
			charListComplete.WriteUInt8(0);
			charListComplete.WriteUInt8(0x01);
			charListComplete.ClosePacket();
			sock.sendPacket(&charListComplete);

			AlefPacket response(0x0D, 0x45, 0x00);
			response.WriteUInt8(0x07);
			response.WriteByteArray("test");
			for (int i = 0; i < 49; i++)
				response.WriteUInt8(0);
			response.ClosePacket();

			sock.sendPacket(&response);*/

	sendDummyCharacter(sock);

	Int8 i8Operation = 7;
	Int32 cID = 0;
	unsigned char acct[] = "acct";

	SharedPtr<AlefPacket> sendCharInfoFinish = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, acct, 0, 0, 0, &cID, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	sock.sendPacket(sendCharInfoFinish);

	return true;
}

bool AlefLoginClientLogin::processCharacterCreation(AlefSocket& sock, AlefPacket * packet)
{
	/*UInt8 operation = packet->GetPacketFlag(FlagIndex::FLAG_IDX1);
	switch (operation)
	{
		case 0x01:
		{
			LOG("Character Creation REQ");
			
			//Send Character Info (02 BF 9F)
			AlefPacket charInfo(0x02, 0xBF, 0x9F);

		} break;
		default:
		{
			stringstream warningMsg;
			warningMsg << "Character Creation Default Case Op: " << (int)operation;
			LOG(warningMsg.str(), WARNING);
		} break;
	}*/
	
	return true;
}

bool AlefLoginClientLogin::processWorldEnterRequest(AlefSocket& sock, AlefPacket * packet)
{
	LOG("World Enter Request");

	//{	Alef::INT8, Alef::CHAR, Alef::INT32, Alef::CHAR, Alef::INT32, Alef::VEC3F, Alef::INT32 }
	Int8 i8Operation = 11;
	Int32 authKey = 12345; //What sorta number is expected here?
	unsigned char name[] = "Dummy#test";
	SharedPtr<AlefPacket> authKeyPkt = pktInterface->buildPacket(Alef::AGSMCHARMANAGER_PACKET_TYPE, &i8Operation, 0, 0, name, 0, 0, &authKey);

	sock.sendPacket(authKeyPkt);

	//Alef::INT32, Alef::CHAR, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::CHAR
	Int32 i32TID = 96;
	memset(name, 0, 10);//clear name
	strcpy((char*)name, "test");
	SharedPtr<AlefPacket> miniCharInfo = pktInterface->buildMiniPacket(Alef::AGPMLOGIN_CHAR_INFO, &i32TID, name, 0, 0, 0, 0, 0, 0, 0);

	//{	Alef::CHAR }
	unsigned char serverAddress[] = "127.0.0.1:11008";
	SharedPtr<AlefPacket> miniServerInfo = pktInterface->buildMiniPacket(Alef::AGPMLOGIN_SERVER_INFO, serverAddress);

	//{	Alef::INT8, Alef::CHAR, Alef::CHAR, Alef::INT8, Alef::CHAR, Alef::INT8, Alef::INT32, Alef::CHAR, Alef::PACKET, Alef::PACKET, Alef::INT32, Alef::PACKET, Alef::CHAR, Alef::CHAR, Alef::INT32, Alef::INT32}
	i8Operation = 8;
	Int32 i32CID = 0;
	SharedPtr<AlefPacket> serverInfoPkt = pktInterface->buildPacket(Alef::AGPMLOGIN_PACKET_TYPE, &i8Operation, 0, 0, 0, 0, 0, &i32CID, 0, miniCharInfo, miniServerInfo, 0, 0, 0, 0, 0, 0);

	sock.sendPacket(serverInfoPkt);

	return true;
}

void AlefLoginClientLogin::sendDummyCharacter(AlefSocket& sock)
{
	Int32 i32Dummy = 100, i32One = 1, i32Zero = 0;
	float fZero = 0;
	Int8 i8Zero = 0;


	//{	Alef::VEC3F, Alef::VEC3F, Alef::INT32, Alef::INT32, Alef::FLOAT, Alef::FLOAT, Alef::INT8, Alef::INT8 }
	Alef::AlefVec3F pos, destPos;
	SharedPtr<AlefPacket> charMovePkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_MOVE, &pos, &destPos, &i32Zero, &i32Zero, &fZero, &fZero, &i8Zero, &i8Zero);

#pragma region FACTORPACKETS

	//{	Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32 }
	SharedPtr<AlefPacket> factorStatusPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_STATUS, &i32Dummy, &i32Dummy, &i32Dummy, &i32Dummy, &i32Dummy, 0, &i32Dummy, &i32Dummy, &i32Dummy, &i32Dummy, 0, &i32Zero, 0);

	//{	Alef::INT32, Alef::INT32, Alef::INT32 }
	SharedPtr<AlefPacket> factorTypePkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_TYPE, &i32One, &i32One, &i32One);

	//{	Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32}
	SharedPtr<AlefPacket> factorPointPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_POINT, &i32Dummy, &i32Dummy, &i32Dummy, &i32Dummy, &i32Dummy, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	//{	Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32 }
	SharedPtr<AlefPacket> factorPointMaxPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_POINTMAX, &i32Dummy, &i32Dummy, &i32Zero, &i32Zero, &i32Zero, 0, 0, 0, 0, 0, 0);

	//Alef::AGPMCHAR_FACTOR_RECOVERY Skipped

	//Alef::AGPMCHAR_FACTOR_ATTRIBUTE Skipped

	//{ Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32 }
	SharedPtr<AlefPacket> factorAttributeEmpty = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_ATTRIBUTE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	//{ Alef::PACKET, Alef::PACKET }
	SharedPtr<AlefPacket> factorDmgPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_DMG, factorAttributeEmpty, factorAttributeEmpty);

	//{ Alef::PACKET, Alef::PACKET }
	SharedPtr<AlefPacket> factorDefPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_DEFENSE, factorAttributeEmpty, factorAttributeEmpty);

	//{ Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32 }
	Int32 i32AtkRng = 50, i32AtkSpd = 60;
	SharedPtr<AlefPacket> factorAttackPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_ATTACK, &i32Dummy, &i32AtkRng, &i32AtkSpd, 0, 0, 0, 0, 0);

	//{ Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32, Alef::INT32 }
	SharedPtr<AlefPacket> factorDirtPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTOR_DIRT, 0, 0, 0, 0, 0, &i32Zero, 0, &i32Zero);

	//Alef::AGPMCHAR_FACTOR_PRICE Skipped

	//{	Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET }
	SharedPtr<AlefPacket> factorResPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTORS, 0, factorStatusPkt, factorTypePkt, 0, factorPointMaxPkt, 0, 0, factorDmgPkt, factorDefPkt, factorAttackPkt, 0, factorDirtPkt, 0);

	//{	Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET, Alef::PACKET }
	SharedPtr<AlefPacket> factorPkt = pktInterface->buildMiniPacket(Alef::AGPMCHAR_FACTORS, factorResPkt, factorStatusPkt, factorTypePkt, factorPointPkt, factorPointMaxPkt, 0, 0, factorDmgPkt, factorDefPkt, factorAttackPkt, 0, factorDirtPkt, 0);

#pragma endregion	

	Int8 i8Operation = 0;
	Int16 i16Zero = 0;
	Int32 charID = 1012, charTID = 96, nameLen = 4;
	Int64 i64Zero = 0;
	unsigned char charName[] = "test", skillInit[33] = { 0 }, signature[] = "Signature";
	/*{	Alef::INT8, Alef::INT32, Alef::INT32, Alef::MEMORY_BLOCK, Alef::INT8, Alef::PACKET, Alef::PACKET, Alef::PACKET,
	Alef::INT64, Alef::INT64, Alef::INT64, Alef::INT8, Alef::INT8, Alef::INT32, Alef::INT8, Alef::UINT8, Alef::UINT8,
	Alef::UINT64, Alef::INT8, Alef::CHAR, Alef::INT8, Alef::INT8, Alef::INT32, Alef::INT8, Alef::UINT16, Alef::INT32,
	Alef::INT32, Alef::CHAR, Alef::MEMORY_BLOCK, Alef::UINT32 }*/
	SharedPtr<AlefPacket> charInfoPkt = pktInterface->buildPacket(Alef::AGPMCHARACTER_PACKET_TYPE,	&i8Operation, &charID, &charTID, &nameLen, charName, &i8Zero, charMovePkt, 0, factorPkt, &i64Zero, &i64Zero, &i64Zero, &i8Zero,
																							&i8Zero, &i32Zero, &i8Zero, &i8Zero, &i8Zero, &i64Zero, &i8Zero, skillInit, &i8Zero, &i8Zero, &i32Zero, &i8Zero, &i16Zero, &i32Zero,
																							&i32Zero, signature, &i16Zero, 0, &i32Zero);

	sock.sendPacket(charInfoPkt);
}