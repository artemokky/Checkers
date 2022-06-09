#include "botV3.h"
#include <array>
#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include <future>

//òàéìåð äëÿ òåñòà ïî ñðàâíåíèþ ñèíõðîííîãî è àñèíõðîííîãî âûïîëíåíèé
class Timer {
public:
	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		auto duration = end - start;
		std::cout << duration.count() << std::endl;
	}
private:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
};

//ñèãíóì
static int sign(int numbr) {
	if (numbr == 0) {
		return 0;
	}
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}


//êîíñòàíòíûé ìàññèâ íåàêòèâíûõ ÿ÷ååê - äëÿ îïòèìèçàöèè ñêîðîñòè ïðîáåãà ïî èãðîâîìó ïîëþ
constexpr std::pair<int, int> NonActiveCells[] = {
	{0,1},{0,3},{0,5},{0,7},{1,0},{1,2},{1,4},{1,6},
	{2,1},{2,3},{2,5},{2,7},{3,0},{3,2},{3,4},{3,6},
	{4,1},{4,3},{4,5},{4,7},{5,0},{5,2},{5,4},{5,6},
	{6,1},{6,3},{6,5},{6,7},{7,0},{7,2},{7,4},{7,6}
};

//êîíñòàíòíûé ìàññèâ àêòèâíûõ ÿ÷ååê - äëÿ îïòèìèçàöèè ñêîðîñòè ïðîáåãà ïî èãðîâîìó ïîëþ
constexpr std::pair<int, int> ActiveCells[] = {
	{1,1},{1,3},{1,5},{1,7},{0,0},{0,2},{0,4},{0,6},
	{3,1},{3,3},{3,5},{3,7},{2,0},{2,2},{2,4},{2,6},
	{5,1},{5,3},{5,5},{5,7},{4,0},{4,2},{4,4},{4,6},
	{7,1},{7,3},{7,5},{7,7},{6,0},{6,2},{6,4},{6,6}
};


//êîíñòðóêòîð áîò
BotV3::BotV3(int b_or_w, Playboard* instance) : Player(instance) {

	//ïðèñâàèâàåì öâåò øàøåê
	color = static_cast<Positions>(b_or_w);

	//èíèöèàëèçèðóåì èãðîâîå ïîëå âíóòðè áîòà - èíèöèàëèçèðóåì íåàêòèâíûå ÿ÷åéêè
	for (auto& nonactcell : NonActiveCells) {
		PosBoard[nonactcell.first][nonactcell.second] = static_cast<int>(Positions::CELL_UNPLBL);
	}

	//èíèöèàëèçèðóåì àêòèâíûå ÿ÷åéêè
	for (auto& actcell : ActiveCells) {
		PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::CELL_PLBL);
	}

	//îïðåäåëÿåì öâåò øàøåê ñîïåðíèêà áîòà
	if (color == Positions::WHITE_CHECKER) {
		enemycolor = Positions::BLACK_CHECKER;
	}
	else
		enemycolor = Positions::WHITE_CHECKER;
}


//ñêàíåð èãðîâîãî ïîëÿ
void BotV3::BoardScanner() {

	//ïðîõîäèì òîëüêî ïî àêòèâíûì ÿ÷åéêàì
	for (auto& actcell : ActiveCells) {

		//îïðåäåëÿåì öâåò íà äàííîé ÿ÷åéêå
		CheckerColor check = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerColor();

		//îïðåäåëÿåì øàøêà ýòî èëè êîðîëü, åñëè ÷òî-òî ñòîèò
		CheckerHierarchy check_post = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerPost();

		//åñëè íè÷åãî íå ñòîèò,
		//òî ýòî ïðîñòî àêòèâíàÿ ÿ÷åéêà
		if (check == EXCEPTION_COLOR) {
			PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::CELL_PLBL);
		}
		else if (check == WHITE) {
			if (check_post == CHECKER) {
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::WHITE_CHECKER);
			}
			else
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::WHITE_KING);
		}
		else if (check == BLACK) {
			if (check_post == CHECKER) {
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::BLACK_CHECKER);
			}
			else
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::BLACK_KING);
		}
	}
}

//ôóíêöèÿ õîäà áîòà
bool BotV3::Turn(int x, int y, CheckerColor b_or_w) {

	//íà íà÷àëî õîäà âûïîëíèë ñêàíèðîâàíèå èãðîâîãî ïîëÿ
	BoardScanner();

	//ïîëó÷èì ñàìûé "ðåéòèíãîâûé" õîä
	PosTurn turn = ReturnMove();


	//âûäåëèì ÿ÷åéêó èç êîòîðîé ïðîèçâåäåí õîä
	gameCheckers->Select(turn.from.first, turn.from.second, b_or_w);
	//ñõîäèì íà ÿ÷åéêó êóäà áûë ïðîèçâåäåí õîä
	gameCheckers->Select(turn.to.first, turn.to.second, b_or_w);

	//åñëè åñòü ìíîæåñòâåííîå ñúåäåíèå, ïðîõîäèìñÿ ïî âåêòîðó ìíîæåñòâåííûõ ñúåäåíèé
	//ïðîæèìàÿ ìåñòà ñúåäåíèé
	if (!turn.another_eats.empty()) {
		for (auto& eats : turn.another_eats) {
			gameCheckers->Select(eats.first, eats.second, b_or_w);
		}
	}

	//îáíóëÿåì âåêòîðà õîäîâ
	RefreshVectors();
	
	//õîä âûïîëíåí
	return true;
}

//îáíîâëåíèå âåêòîðîâ õîäîâ
void BotV3::RefreshVectors() {
	PosMoves.clear();
	PosMovesEnemy.clear();
	//ïåðåíîñèì â íà÷àëüíîå ñîñòîÿíèå ôëàã ñúåäåíèÿ âðàãîì
	canEnemyEat = false;
}


//ïðîâåðêà ãðàíèö õîäà
inline bool BotV3::CheckBorders(int x, int y, int dx, int dy) {
	return x + dx >= 0 && x + dx < 8 && y + dy >= 0 && y + dy < 8;
}

//ïðîâåðêà õîäà íà ñòàíîâëåíèå äàìêîé
inline bool BotV3::CheckBecomeKing(int y, Positions Col) {
	if (Col == Positions::WHITE_CHECKER) {
		//äëÿ áåëûõ - åñëè äîñòèãíóòà âåðõíÿÿ ãðàíèöà ïîëÿ
		if (y == 7) {
			return true;
		}
	}
	else if (Col == Positions::BLACK_CHECKER) {
		//äëÿ ÷åðíûõ - åñëè äîñòèãíóòà íèæíÿÿ ãðàíèöà ïîëÿ
		if (y == 0) {
			return true;
		}
	}
	return false;
}


/*ôóíêöèÿ íàõîæäåíèÿ âîçìîæíûõ ñúåäåíèé îáû÷íîé øàøêîé â îäíó ñòîðîíó, ñòîðîíà îïðåäåëÿåòñÿ ïåðåìåííûìè dx dy
* Board - ïîëå íà êîòîðîì ïðîèñõîäèò íàõàæîäåíèå õîäà
* x, y - îòêóäà
* dx dy - ïðèðàùåíèå êîîðäèíàòû
* Col - öâåò áîòà
* enemyCol - öâåò ñîïåðíèêà
* movesVec - âåêòîð âîçìîæíûõ õîäîâ
*/
void BotV3::CheckEats(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//ïðîâåðèì ãðàíèöû
	if (CheckBorders(x, y, dx, dy)) {

		//ïðîâåðèì ñîñåäíþþ ÿ÷åéêó â ñîîòâåòñòâèè ñ âåêòîðîì ïðèðàùåíèÿ íà íàëè÷èå øàøêè ñîïåðíèêà
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//ïðîâåðèì ÿ÷åéêó, íà êîòîðóþ áóäåì ïåðåìåùàòüñÿ â ñëó÷àå ñúåäåíèÿ, íàõîäèòñÿ ëè îíà â ãðàíèöàõ ïîëÿ è ïóñòàÿ ëè îíà
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//åñëè âñå óñëîâèÿ âûïîëíåíû
				// 
				//ñîçäàåì êîîðäèíàòû
				//xy - îòêóäà ïåðåìåùàåìñÿ
				//eatedxy - ÷òî ñúåëè
				//dxdy - êóäà ïåðåìåñòèëèñü
				auto xy = std::make_pair(x, y);
				auto eatedxy = std::make_pair(x + dx, y + dy);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//çàïîëíÿåì âåêòîð õîäîâ è çàïîëíÿåì âåêòîð ñúåäåííûõ øàøåê çà ýòîò õîä
				movesVec.push_back({ xy, dxdy });
				movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);


				//ïîâûøàåì ðåéòèíã õîäà
				//ïîâûøàåì íà ðåéòèíã ñúåäåíèÿ
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);

				//åñëè ñúåëè äàìêó, òî ïîâûøàåì íà öåííîñòü ñúåäåíèÿ äàìêè
				if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
				}
				//åñëè øàøêà ñòàíåò äàìêîé â ñëó÷àå ýòîãî õîäà, òî ïîâûøàåì ðåéòèíã íà âåëè÷èíó ñòàíîâëåíèÿ äàìêîé
				if (CheckBecomeKing(dxdy.second, Col)) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
				}
			}
		}
	}
}


/*ôóíêöèÿ íàõîæäåíèÿ âîçìîæíûõ ñúåäåíèé äàìêîé â îäíó ñòîðîíó, ñòîðîíà îïðåäåëÿåòñÿ ïåðåìåííûìè dx dy
* Board - ïîëå íà êîòîðîì ïðîèñõîäèò íàõàæîäåíèå õîäà
* x, y - îòêóäà
* dx dy - ïðèðàùåíèå êîîðäèíàòû
* Col - öâåò áîòà
* enemyCol - öâåò ñîïåðíèêà
* movesVec - âåêòîð âîçìîæíûõ õîäîâ
*/
void BotV3::CheckEatsForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//îïðåäåëÿåì çíàê ïðèðàùåíèÿ
	int ddx = sign(dx);
	int ddy = sign(dy);

	//ïðîâåðÿåì õîäû, ïîêà íàõîäèìñÿ â ãðàíèöàõ
	while (CheckBorders(x, y, dx, dy)) {

		//ïðîâåðèì ñîñåäíþþ ÿ÷åéêó â ñîîòâåòñòâèè ñ âåêòîðîì ïðèðàùåíèÿ íà íàëè÷èå øàøêè ñîïåðíèêà
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//ïðîâåðèì ÿ÷åéêó, íà êîòîðóþ áóäåì ïåðåìåùàòüñÿ â ñëó÷àå ñúåäåíèÿ, íàõîäèòñÿ ëè îíà â ãðàíèöàõ ïîëÿ è ïóñòàÿ ëè îíà
			if (CheckBorders(x, y, dx + ddx, dy + ddy) && Board[x + dx + ddx][y + dy + ddy] == static_cast<int>(Positions::CELL_PLBL)) {
				
				//åñëè ìîæåì ñúåñòü øàøêó, òî äîáàâëÿåì å¸ êîîðäèíàòû â âåêòîð ñúåäàåìûõ øàøåê
				auto eatedxy = std::make_pair(x + dx, y + dy);

				//ïåðåõîäèì íà äàííóþ êëåòêó
				dx += ddx;
				dy += ddy;

				//òåïåðü äîáàâèì âñå âîçìîæíûå õîäû â îäíó ñòîðîíó äëÿ äàìêè ïîñëå ñúåäåíèÿ
				while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

				    //åñëè âñå óñëîâèÿ âûïîëíåíû
				    // 
				    //ñîçäàåì êîîðäèíàòû
				    //xy - îòêóäà ïåðåìåùàåìñÿ
				    //dxdy - êóäà ïåðåìåñòèëèñü
					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + dx, y + dy);
					movesVec.push_back({ xy, dxdy });
					movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);

					//åñëè ìîæåì ñúåñòü ïîâûøàåì ðåéòèíã
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);
					//åñëè ñúåäàåì äàìêó òî åù¸ ïîâûøàåì ðåéòèíã õîäà
					if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
						movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
					}
					//ïåðåäâèãàåìñÿ äàëüøå
					dx += ddx;
					dy += ddy;
				}
				//return â ñëó÷àå, åñëè ìû íà÷àëè âûõîäèòü çà ãðàíèöû èëè ÿ÷åéêà çàíÿòà
				return;
			}
			else
				return;
		}
		//ïåðåäâèãàåìñÿ äàëüøå ïîêà íå âñòðåòèì øàøêó ñîïåðíèêà
		dx += ddx;
		dy += ddy;
	}

}

//íàéäåì âñå âîçìîæíûå ñúåäåíèÿ äëÿ âñåõ øàøåê
void BotV3::FindEats(std::array<std::array<int, 8>, 8>& Board, Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {
	//ïðîõîäèìñÿ ïî âñåì ÿ÷åéêàì(íå îïòèìèçèðîâàíî)
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {

			//åñëè öâåò ÿ÷åéêè ñîâïàäàåò ñ çàäàííûì, òî ïðîâåðÿåì íà ñúåäåíèå âî âñå 4 ñòîðîíû
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckEats(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
			//åñëè öâåò ÿ÷åéêè ñîâïàäàåò ñ íàøèì, íî ÿâëÿåòñÿ äàìêîé, òî ïðîâåðÿåì íà ñúåäåíèå äàìêîé âî âñå 4 ñòîðîíû
			if (Board[x][y] == static_cast<int>(Col) + 2) {
				CheckEatsForKing(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
		}
	}
}


/*ôóíêöèÿ íàõîæäåíèÿ âîçìîæíûõ õîäîâ îáû÷íîé øàøêîé âî âñå ñòîðîíû, ñòîðîíû îïðåäåëÿþòñÿ öâåòîì øàøêè
* Board - ïîëå íà êîòîðîì ïðîèñõîäèò íàõàæîäåíèå õîäà
* x, y - îòêóäà
* Col - öâåò áîòà
* movesVec - âåêòîð âîçìîæíûõ õîäîâ
*/
void BotV3::CheckMoves(std::array<std::array<int, 8>, 8>& Board, int x, int y, Positions Col, std::vector<PosTurn>& movesVec) {
	//ïåðåìåííûå ïåðåìåùåíèÿ
	int dx = 1, dy;

	//åñëè áåëûå - èäåì ââåðõ, ñ ÷åðíûìè èíà÷å
	if (Col == Positions::WHITE_CHECKER) {
		dy = 1;
	}
	else
		dy = -1;

	//ñíà÷àëà âïðàâî
	//ïðîâåðÿåì ãðàíèöû
	if (CheckBorders(x, y, dx, dy)) {
		
		//åñëè ÿ÷åéêà ïóñòàÿ, ìîæåì ñõîäèòü íà íå¸
		if (Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

			//ñîçäàåì êîîðäèíàòû õîäà, ïåðâîå - îòêóäà, âòîðîå - êóäà
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });

			//åñëè ìîæåì ñõîäèòü òî äîáàâëÿåì ñîîòâåòñòâóþùåå çíà÷åíèå
			movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);

			//åñëè ñòàíîâèìñÿ äàìêîé, òî äîáàâëÿåì ñîîòâåòñòâóþùèé ðåéòèíã
			if (CheckBecomeKing(dxdy.second, Col)) {
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
			}
		}
	}

	//òåïåðü òî æå ñàìîå òîëüêî âëåâî
	dx = -1;

	if (CheckBorders(x, y, dx, dy)) {
		if (PosBoard[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });
			movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);
			if (CheckBecomeKing(dxdy.second, Col)) {
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
			}
		}
	}
}


/*ôóíêöèÿ íàõîæäåíèÿ âîçìîæíûõ õîäîâ äàìêîé â îäíó ñòîðîíó, ñòîðîíà îïðåäåëÿåòñÿ ïåðåìåííûìè dx dy
* Board - ïîëå íà êîòîðîì ïðîèñõîäèò íàõàæîäåíèå õîäà
* x, y - îòêóäà
* dx dy - ïðèðàùåíèå êîîðäèíàòû
* movesVec - âåêòîð âîçìîæíûõ õîäîâ
*/
void BotV3::CheckMovesForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, std::vector<PosTurn>& movesVec) {
	//îïðåäåëÿåì çíàê ïðèðàùåíèÿ
	int ddx = sign(dx);
	int ddy = sign(dy);

	//ïðîâåðÿåì ãðàíèöû è îòñóòñòâèå øàøåê íà êëåòêå
	while (CheckBorders(x, y, dx, dy) && PosBoard[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) { //è ïîêà íåò øàøåê íà êëåòêå

		//äîáàâèì êîîðäèíàòû îòêóäà-êóäà â âåêòîð õîäà
		auto xy = std::make_pair(x, y);
		auto dxdy = std::make_pair(x + dx, y + dy);
		movesVec.push_back({ xy, dxdy });
		movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);
		dx += ddx;
		dy += ddy; //ïåðåäâèãàåìñÿ âïåðåä
	}
}


/*ôóíêöèÿ íàõîæäåíèÿ âñåõ âîçìîæíûõ õîäîâ
* Board - ïîëå íà êîòîðîì ïðîèñõîäèò íàõàæîäåíèå õîäà
* Col - öâåò áîòà
* movesVec - âåêòîð âîçìîæíûõ õîäîâ
*/
void BotV3::FindMoves(std::array<std::array<int, 8>, 8>& Board, Positions Col, std::vector<PosTurn>& movesVec) {

	//ïðîõîäèìñÿ ïî âñåìó ïîëþ
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			//åñëè îáû÷íàÿ øàøêà
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckMoves(Board, x, y, Col, movesVec);
			}
			//åñëè äàìêà
			else if (Board[x][y] == static_cast<int>(Col) + 2) {
				//ïðîõîäèì âî âñå ñòîðîíû
				CheckMovesForKing(Board, x, y, 1, 1, movesVec);
				CheckMovesForKing(Board, x, y, -1, 1, movesVec);
				CheckMovesForKing(Board, x, y, 1, -1, movesVec);
				CheckMovesForKing(Board, x, y, -1, -1, movesVec);
			}
		}
	}
}


//ïðîâåðêà âåêòîðà ñúåäåííûõ øàøåê, áûëà ëè ñúåäåíà øàøêà ïî äàííîé êîîðäèíàòå
bool BotV3::IsWasChecked(int x, int y, std::vector<std::pair<int, int>>& enemyPos) {
	for (auto& enemy : enemyPos) {
		//åñëè êîîðäèíàòû øàøêè è çàäàííûõ êîîðäèíàò ñîâïàäàþò òî áûëà ñúåäåíà
		if (enemy.first == x && enemy.second == y) {
			return true;
		}
	}
	//èíà÷å íåò
	return false;
}


bool BotV3::IsWasCheckedByFather(int x, int y, ternaryTree* wayTree) {
	ternaryTree* helpTree = wayTree;
	while (helpTree->father != nullptr && helpTree->father->father != nullptr) {
		if (helpTree->father->getXto() == x && helpTree->father->getYto() == y) {
			return false;
		}
		helpTree = helpTree->father;
	}


}


/*ôóíêöèÿ ïðîâåðêè âîçìîæíûõ ñúåäåíèé ïîñëå òîãî êàê ñúåëè øàøêó
* Board - èãðîâîå ïîëå
* wayTree - òðîè÷íîå äåðåâî îïòèìàëüíîãî ïóòè
* enemyCol - öâåò ñîïåðíèêà
* stop - êîîðäèíàòà îñòàíîâêè, ÷òîáû íå õîäèòü ïî êðóãó
* enemyPos - âåêòîð ñúåäåííûõ øàøåê
*/
void BotV3::CheckContinue
(std::array<std::array<int, 8>, 8>& Board, ternaryTree* wayTree, Positions enemyCol,
	std::pair<int, int> stop, std::vector<std::pair<int, int>>& enemyPos) {

	//ñîçäàäèì ïåðåìåííûå äëÿ óäîáñòâà
	int dx = wayTree->getDX();
	int dy = wayTree->getDY();
	int x = wayTree->getXto();
	int y = wayTree->getYto();

	//ïðîâåðêà ãðàíèö
	if (CheckBorders(x, y, dx, dy)) {

		//ïðîâåðèì ñîñåäíþþ ÿ÷åéêó â ñîîòâåòñòâèè ñ âåêòîðîì ïðèðàùåíèÿ íà íàëè÷èå øàøêè ñîïåðíèêà
		if ((Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)

			//ïðîâåðèì áûëà ëè ñúåäåíà äàííàÿ øàøêà è íå ÿâëÿåòñÿ ëè äàííàÿ êîîðäèíàòà êîîðäèíàòîé îñòàíîâêè
			&& !IsWasChecked(x + dx, y + dy, enemyPos) && (x + dx != stop.first && y + dy != stop.second)) {

			//ïðîâåðÿåì ãðàíèöû ÿ÷åéêè, êóäà áóäåì ïåðåìåùàòüñÿ ïîñëå ñúåäåíèÿ
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {


				//äîáàâèì êîîðäèíàòû îòêóäà-êóäà â äåðåâî ïóòè
				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//ïîâûñèì âûñîòó äåðåâà
				wayTree->incrementHeight();
				wayTree->first = new ternaryTree(xy, dxdy, wayTree);

				//ïðîâåðèì âîçìîæíîñòü äàëüíåéøåãî ñúåäåíèÿ äëÿ ýòîé âåòêè ïóòè
				CheckContinue(Board, wayTree->first, enemyCol, stop, enemyPos);

			}
		}
	}
	//òî æå ñàìîå ÷òî è ñâåðõó òîëüêî äëÿ äðóãîãî íàïðàâëåíèÿ
	if (CheckBorders(x, y, dx, -dy)) {

		if ((Board[x + dx][y - dy] == static_cast<int>(enemyCol) || Board[x + dx][y - dy] == static_cast<int>(enemyCol) + 2)
			&& !IsWasChecked(x + dx, y - dy, enemyPos) && (x + dx != stop.first && y - dy != stop.second)) {

			if (CheckBorders(x, y, 2 * dx, -2 * dy) && Board[x + 2 * dx][y - 2 * dy] == static_cast<int>(Positions::CELL_PLBL)
				&& !(x + 2 * dx == stop.first && y - 2 * dy == stop.second) &&
				//äîáàâëÿåòñÿ åù¸ îäíà ïðîâåðêà, áûë ëè ïðîâåðåí äàííûé õîä ïðåäêàìè
				//ñîçäàíî äëÿ òîãî, ÷òîáû íå õîäèòü ïî êðóãó
				//â ïåðâîì ñëó÷àå ýòîãî íå ïðîèñõîäèò, òàê êàê â ñëó÷àå ìíîãîêðàòíîãî ñúåäåíèÿ,
				//ïåðåäâèæåíèå â îäíîì íàïðàâëåíèè íå äàñò çàöèêëèâàíèÿ
				IsWasCheckedByFather(x + 2 * dx, y - 2 * dy, wayTree)) {

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y - 2 * dy);


				wayTree->incrementHeight();
				wayTree->second = new ternaryTree(xy, dxdy, wayTree);


				CheckContinue(Board, wayTree->second, enemyCol, stop, enemyPos);
			}
		}
	}
	//òî æå ñàìîå
	if (CheckBorders(x, y, -dx, dy)) {

		if ((Board[x - dx][y + dy] == static_cast<int>(enemyCol) || Board[x - dx][y + dy] == static_cast<int>(enemyCol) + 2)
			&& !IsWasChecked(x - dx, y + dy, enemyPos)) {

			if (CheckBorders(x, y, -2 * dx, 2 * dy) && Board[x - 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)
				&& !(x - 2 * dx == stop.first && y + 2 * dy == stop.second) &&
				IsWasCheckedByFather(x - 2 * dx, y + 2 * dy, wayTree)) {

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x - 2 * dx, y + 2 * dy);


				wayTree->incrementHeight();
				wayTree->third = new ternaryTree(xy, dxdy, wayTree);


				CheckContinue(Board, wayTree->third, enemyCol, stop, enemyPos);

			}
		}
	}
}



//íàõîäèì îïòèìàëüíûé õîä, â ñëó÷àå íàëè÷èÿ ìíîæåñòâåííûõ õîäîâ è ðàçâèëîê
//ïîäáèðàåòñÿ ñíà÷àëà èñêëþ÷åíèåì
//ïîòîì ó êîãî âûñîòà áîëüøå
void BotV3::FillAnotherEats(ternaryTree* wayTree, PosTurn& pos) {
	if (wayTree->first == nullptr && wayTree->second == nullptr && wayTree->third == nullptr) {
		return;
	}
	else if (wayTree->first != nullptr && wayTree->second == nullptr && wayTree->third == nullptr) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);
	}
	else if (wayTree->second != nullptr && wayTree->first == nullptr && wayTree->third == nullptr) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->third != nullptr && wayTree->second == nullptr && wayTree->first == nullptr) {
		pos.another_eats.push_back(wayTree->third->_to);
		FillAnotherEats(wayTree->third, pos);
	}
	else if (wayTree->third == nullptr) {
		if (wayTree->first->height > wayTree->second->height) {
			pos.another_eats.push_back(wayTree->first->_to);
			FillAnotherEats(wayTree->first, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->second->_to);
			FillAnotherEats(wayTree->second, pos);

		}
	}
	else if (wayTree->second == nullptr) {
		if (wayTree->first->height > wayTree->third->height) {
			pos.another_eats.push_back(wayTree->first->_to);
			FillAnotherEats(wayTree->first, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->third->_to);
			FillAnotherEats(wayTree->third, pos);

		}
	}
	else if (wayTree->first == nullptr) {
		if (wayTree->second->height > wayTree->third->height) {
			pos.another_eats.push_back(wayTree->second->_to);
			FillAnotherEats(wayTree->second, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->third->_to);
			FillAnotherEats(wayTree->third, pos);
		}
	}
	else if (wayTree->first->height > wayTree->second->height && wayTree->first->height > wayTree->third->height) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);

	}
	else if (wayTree->second->height > wayTree->first->height && wayTree->second->height > wayTree->third->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->third->height > wayTree->first->height && wayTree->third->height > wayTree->second->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->first->height == wayTree->second->height) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);
	}
	else if (wayTree->second->height == wayTree->third->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
}


//ôóíêöèÿ âîçâðàùàþùàÿ èíäåêñ íàïðàâëåíèÿ, ïî êîòîðîìó áóäåò ïðîëîæåí ïóòü
inline Ways getWay(int dx, int dy) {
	if (dx == 1 && dy == 1) {
		return FIRST;
	}
	else if (dx == -1 && dy == 1) {
		return SECOND;
	}
	else if (dx == 1 && dy == -1) {
		return THIRD;
	}
	else if (dx == -1 && dy == -1) {
		return FOURTH;
	}
	return FATHER;
}


//ôóíêöèÿ íàõîæäåíèÿ äîïîëíèòåëüíûõ ñúåäåíèé äàìêîé
void BotV3::FindWay(int x, int y, int dx, int dy, std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {

	//ïî âåêòîðó íàïðàâëåíèÿ âûáèðàåì ïóòü
	auto destiny = getWay(dx, dy);

	//ïðîâåðÿåì ãðàíèöû ïîëÿ
	while (CheckBorders(x, y, dx, dy)) {

		//ïðîâåðÿåì ñíà÷àëà íà òî, åñòü ëè øàøêà íà ïóòè
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)
		{
			//åñëè åñòü òî ïðîâåðÿåì ñâîáîäíî ëè çà íåé
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//ìåíÿåì ìåñòàìè øàøêó 
				std::swap(Board[x][y], Board[x + 2 * dx][y + 2 * dy]);

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				Board[x + dx][y + dy] = static_cast<int>(Positions::CELL_PLBL);


				//çàïîëíÿåì äåðåâî ïóòè íîâûì õîäîì
				wayTree->incrementHeight();
				wayTree->way[destiny] = new quadrupleTree(xy, dxdy, wayTree);
				wayTree->way[destiny]->eated = std::make_pair(x + dx, y + dy);

				//ðåêóðñèÿ
				CheckContinueForKing(Board, wayTree->way[destiny], enemyCol);

			}
		}
		//åñëè íåò, òî èäåì äàëüøå â ïîèñêå øàøêè
		x += dx;
		y += dy;
	}
}


//ôóíêöèÿ, íàõîäÿùàÿ âîçìîæíûå äîïîëíèòåëüíûå ñúåäåíèÿ äëÿ äàìêè âî âñå 4 ñòîðîíû
void BotV3::CheckContinueForKing
(std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {
	int dx = sign(wayTree->getDX());
	int dy = sign(wayTree->getDY());
	int x = wayTree->getXto();
	int y = wayTree->getYto();

	FindWay(x, y, dx, dy, Board, wayTree, enemyCol);
	FindWay(x, y, -dx, dy, Board, wayTree, enemyCol);
	FindWay(x, y, dx, -dy, Board, wayTree, enemyCol);
	FindWay(x, y, -dx, -dy, Board, wayTree, enemyCol);
}


//ôóíêöèÿ, âûáèðàþùàÿ îïòèìàëüíûé ïóòü ñúåäåíèÿ øàøåê, äëÿ òîãî ÷òîáû ñúåñòü ïî ìàêñèìóìó
void BotV3::FillAnotherEatsForKing(quadrupleTree* wayTree, PosTurn& pos) {
	int height = -1;

	//ñíà÷àëà íàõîäèì ìàêñèìàëüíóþ âûñîòó äåðåâà
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height > height) {

			height = wayTree->way[i]->height;

		}
	}

	//çàòåì äëÿ ìàêñèìàëüíîé âûñîòû çàïèõèâàåì â âåêòîð äîï õîäîâ íàø íîâûé äîïîëíèòåëüíûé õîä ñúåäåíèÿ
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height == height) {

			pos.another_eats.push_back(wayTree->way[i]->_to);
			pos.whoWasEated.push_back(wayTree->way[i]->eated);
			FillAnotherEatsForKing(wayTree->way[i], pos);
		}
	}
}

//ôóíêöèÿ íàõîæäåíèÿ ìíîæåñòâåííîãî ñúåäåíèÿ äëÿ êàæäîãî õîäà, âûçûâàåòñÿ ïîñëå òîãî, êàê áûëè íàéäåíû õîäû äëÿ ñúåäåíèÿ
void BotV3::FindContinue(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {

	std::vector<PosTurn> newMoves;

	//äëÿ êàæäîãî õîäà ñúåäåíèÿ
	for (auto& pos : movesVec) {

		//ïðîâåðÿåì îáû÷íîé øàøêîé áûë ñäåëàí õîä èëè äàìêîé
		if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col)) {

			//ñîçäàåì äåðåâî ïóòè
			ternaryTree* wayTree = new ternaryTree(pos.from, pos.to);

			//ñúåäåííûå øàøêè
			std::vector<std::pair<int, int>> enemyPos;

			//äîáàâëÿåì ñúåäåííóþ øàøêó â âåêòîð
			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);

			//âûïîëíÿåì ôóíêöèþ íàõîæäåíèÿ ïóòåé ñúåäåíèÿ øàøåê
			CheckContinue(Board, wayTree, enemyCol, pos.from, enemyPos);

			//çàïîëíÿåì ñàìûì áîëüøèì ïî ãëóáèíå ïóòåì âåêòîð ïóòè
			FillAnotherEats(wayTree, pos);


			//çäåñü çàïîëíÿåì âåêòîð âñåõ ñúåäåííûõ øàøåê
			if (!pos.another_eats.empty()) {
				int dx = (pos.to.first - pos.another_eats[0].first) / 2;
				int dy = (pos.to.second - pos.another_eats[0].second) / 2;
				int x = pos.another_eats[0].first + dx;
				int y = pos.another_eats[0].second + dy;

				pos.whoWasEated.push_back(std::make_pair(x, y));


				for (int i = 0; i < pos.another_eats.size() - 1; i++) {
					int dx = (pos.another_eats[i + 1].first - pos.another_eats[i].first) / 2;
					int dy = (pos.another_eats[i + 1].second - pos.another_eats[i].second) / 2;
					int x = pos.another_eats[i].first + dx;
					int y = pos.another_eats[i].second + dy;

					pos.whoWasEated.push_back(std::make_pair(x, y));
				}

				//îöåíèâàåì öåííîñòü õîäà, â çàèâèìîñòè îò êîëè÷åñòâà ñúåäåííûõ øàøåê
				pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);
			}
		}
		else if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col) + 2) {


			//ñîçäàåì ÷åòâåðíîå äåðåâî
			quadrupleTree* wayTree = new quadrupleTree(pos.from, pos.to);

			std::vector<std::pair<int, int>> enemyPos;

			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);


			//ïîïûòàåìñÿ íàéòè ïðîäîëæåíèå õîäà äëÿ äàìêè êîòîðàÿ ñúåëà
			CheckContinueForKing(Board, wayTree, enemyCol);

			FillAnotherEatsForKing(wayTree, pos);

			//åñëè ñúåëà òî óâåëè÷èâàåì öåííîñòü õîäà â çàèâèñèìîñòè îò êîë-âà ñúåäåííûõ øàøåê
			pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);


			//çäåñü ìû íàõîäèì äîïîëíèòåëüíûå õîäû ïîñëå ñúåäåíèÿ
			//òàê êàê ïîñëå òîãî êàê ìû ñúåëè ïîñëåäíþþ øàøêó
			//îñòàþòñÿ äîïîëíèòåëüíûå õîäû äëÿ ïåðåìåùåíèÿ
			if (!pos.another_eats.empty()) {

				//îïðåäåëÿåì ïîçèöèþ äàìêè â òåêóùèé ìîìåíò ïîñëå ïîñëåäíåãî ñúåäåíèÿ
				int x = pos.another_eats[pos.another_eats.size() - 1].first;
				int y = pos.another_eats[pos.another_eats.size() - 1].second;

				//îïðåäåëÿåì âåêòîð, ïî êîòîðîìó äàìêà äâèãàëàñü âî âðåìÿ ïîñëåäíåãî ñúåäåíèÿ
				int dx = x - pos.whoWasEated[pos.whoWasEated.size() - 1].first;
				int dy = y - pos.whoWasEated[pos.whoWasEated.size() - 1].second;


				//çàïîëíÿåì âåêòîð äîïîëíèòåëüíûõ õîäîâ, êîïèðóÿ òåêóùèé è èçìåíÿÿ ó íåãî ïîñëåäíèé õîä
				while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {
					PosTurn nextpos = pos;
					nextpos.another_eats[nextpos.another_eats.size() - 1].first = x + dx;
					nextpos.another_eats[nextpos.another_eats.size() - 1].second = y + dy;
					newMoves.push_back(nextpos);
					dx += dx;
					dy += dy;
				}
			}

		}

	}

	//çàïîëíÿåì âåêòîð õîäîâ ñîäåðæèìûì âåêòîðà äîïîëíèòåëüíûõ õîäîâ
	if (!newMoves.empty()) {
		for (int i = 0; i < newMoves.size() - 1; i++) {
			movesVec.push_back(newMoves[i]);
		}
	}
}


//ôóíêöèÿ çàïîëíåíèÿ âåêòîðà õîäîâ âñåâîçìîæíûìè õîäàìè
void BotV3::FillPosTurns(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {


	//íàéäåì âñå âîçìîæíûå õîäû äëÿ ñúåäåíèÿ
	FindEats(Board, Col, enemyCol, movesVec);

	//åñëè õîäîâ äëÿ ñúåäåíèÿ íåò, èùåì îáû÷íûå õîäû
	if (movesVec.empty()) {
		FindMoves(Board, Col, movesVec);
	}
	else {
		//åñëè õîäû äëÿ ñúåäåíèÿ åñòü, èçìåíÿåì ôëàã òîãî, ÷òî áûëè ñúåäåíû øàøêè
		if (enemyCol == color) {
			canEnemyEat = true;
		}
		//íàõîäèì äîïîëíèòåëüíûå ìíîæåñòâåííûå ñúåäåíèÿ
		FindContinue(Board, movesVec, Col, enemyCol);
	}
}

//ôóíêöèÿ ïåðåìåùåíèÿ øàøêè ñîïåðíèêà
void BotV3::TryMoveEnemy
(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, PosTurn& pos, PosTurn& enemyTurn, Positions Col, Positions enemyCol) {


	//åñëè ó ñîïåðíèêà åñòü ìíîæåñòâåííîå ñúåäåíèå
	if (!enemyTurn.another_eats.empty()) {
		//òî ìåíÿåì ìåñòàìè ÿ÷åéêó îòïðàâëåíèÿ è ñàìóþ ïîñëåäíþþ ÿ÷åéêó
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second],
			Board[enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].first][enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].second]);
	}
	else {
		//åñëè ìíîæåñòâåííîãî ñúåäåíèÿ íåò, òî ïðîñòî ìåíÿåì ÿ÷åéêó îòêóäà ìåñòàìè ñ ÿ÷åéêîé êóäà
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second], Board[enemyTurn.to.first][enemyTurn.to.second]);
	}

	//åñëè áûëè ñúåäåíèÿ
	if (!enemyTurn.whoWasEated.empty()) {
		for (auto& eated : enemyTurn.whoWasEated) {
			//òî äëÿ êàæäîé ÿ÷åéêè, ãäå áûëà ñúåäåíà øàøêà âûñòàâëÿåì ïóñòóþ àêòèâíóþ ÿ÷åéêó
			Board[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	std::vector<PosTurn> movesVecMy;

	//òåïåðü íàõîäèì âîçìîæíûå ñúåäåíèÿ äëÿ ìîèõ øàøåê
	FindEats(Board, Col, enemyCol, movesVecMy);

	//åñëè îí ïóñòîé
	if (movesVecMy.empty()) {
		//òî ïðîñòî íàõîäèì âîçìîæíûå õîäû
		FindMoves(Board, Col, movesVecMy);

	}
	else {
		//åñëè ìîæåì ñúåñòü, òî äîáàâëÿåì ðåéòèíã ê äàííîìó õîäó ñ ïîíèæåííûì êîýôôèöèåíòîì
		//òàê êàê âåðîÿòíîñòü äàííîãî ñîáûòèÿ îòíîñèòåëüíî ìàëà
		pos.relevance += static_cast<int>(SituationCost::EAT) * movesVecMy.size() / 5;

		//íàõîäèì ìíîæåñòâåííîå ñúåäåíèå
		FindContinue(Board, movesVecMy, enemyCol, Col);

		for (auto& moves : movesVecMy) {


			if (!moves.another_eats.empty()) {
				//åñëè ìîæåì ñúåñòü, òî äîáàâëÿåì ðåéòèíã ê äàííîìó õîäó ñ ïîíèæåííûì êîýôôèöèåíòîì
				pos.relevance += static_cast<int>(SituationCost::EAT_MORE) * moves.another_eats.size() / 5;

			}


		}
	}
}


//ôóíêöèÿ ïåðåìåùåíèÿ è îöåíêè õîäà íà òåñòîâîì ïîëå
void BotV3::TryMove(PosTurn& pos, Positions Col, Positions enemyCol) {

	//ñîçäàåì òåñòîâîå ïîëå
	auto testBoard = PosBoard;

	//åñëè åñòü ìíîæåñòâåííîå ñúåäåíèå
	if (!pos.another_eats.empty()) {

		//òî ìåíÿåì ìåñòàìè ÿ÷åéêó îòïðàâëåíèÿ è ñàìóþ ïîñëåäíþþ ÿ÷åéêó
		std::swap(testBoard[pos.from.first][pos.from.second],
			testBoard[pos.another_eats[pos.another_eats.size() - 1].first][pos.another_eats[pos.another_eats.size() - 1].second]);
	}
	else {

		//åñëè ìíîæåñòâåííîãî ñúåäåíèÿ íåò, òî ïðîñòî ìåíÿåì ÿ÷åéêó îòêóäà ìåñòàìè ñ ÿ÷åéêîé êóäà
		std::swap(testBoard[pos.from.first][pos.from.second], testBoard[pos.to.first][pos.to.second]);
	}

	//åñëè áûëè ñúåäåíèÿ
	if (!pos.whoWasEated.empty()) {
		for (auto& eated : pos.whoWasEated) {
			//òî äëÿ êàæäîé ÿ÷åéêè, ãäå áûëà ñúåäåíà øàøêà âûñòàâëÿåì ïóñòóþ àêòèâíóþ ÿ÷åéêó
			testBoard[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	//âåêòîð õîäîâ ñîïåðíèêà
	std::vector<PosTurn> movesVecEnemy;

	//íàéäåì õîäû ñúåäåíèÿ äëÿ ñîïåðíèêà
	FindEats(testBoard, enemyCol, Col, movesVecEnemy);


	//åñëè õîäû ïóñòûå
	if (movesVecEnemy.empty()) {
		if (canEnemyEat == true) {
			//äîáàâëÿåì ðàçíèöó â âîçìîæíûõ ñúåäåíèÿõ óìíîæåííûõ íà êîýôôèöèåíò ñîõðàíåíèÿ øàøêè
			pos.relevance += static_cast<int>(SituationCost::SAVE) * PosMovesEnemy.size();
		}

		//íàõîäèì âîçìîæíûå õîäû
		FindMoves(testBoard, Col, movesVecEnemy);

		//äîáàâëÿåì ðàçíèöó â âîçìîæíûõ õîäàõ äî è ïîñëå íàøåãî õîäà ïîìíîæåííîãî íà êîýôôèöèåíò áëîêèðîâêè õîäà
		pos.relevance += static_cast<int>(SituationCost::BLOCK) * (PosMovesEnemy.size() - movesVecEnemy.size());
	}
	else {

		//åñëè ñúåäåíèå áûëî, òî ýòîò õîä íå òàê õîðîø, è óáàâëÿåì ðåéòèíã â çàâèñèìîñòè îò ÷èñëà õîäîâ, êîòîðûå ìîãóò åñòü
		pos.relevance += static_cast<int>(SituationCost::DIE) * movesVecEnemy.size();

		//íàõîäèì ìíîæåñòâåííûå ñúåäåíèÿ
		FindContinue(testBoard, movesVecEnemy, enemyCol, Col);

		for (auto& moves : movesVecEnemy) {


			if (!moves.another_eats.empty()) {
				//åñëè ìíîæåñòâåííûå ñúåäåíèÿ åñòü, òî òàê æå èçìåíÿåì ðåéòèíã â õóäøóþ ñòîðîíó
				pos.relevance += static_cast<int>(SituationCost::DIE_MORE) * moves.another_eats.size();

			}


		}


	}

	//òåïåðü ïðîèçâîäèì ïåðåäâèæåíèå êàæäûì õîäîì ñîïåðíèêà
	for (auto& enemyTurn : movesVecEnemy) {
		TryMoveEnemy(testBoard, movesVecEnemy, pos, enemyTurn, Col, enemyCol);
	}
}

void BotV3::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//ïûòàåìñÿ ñõîäèòü êàæäûì õîäîì, ÷òîáû ïðîèçâåñòè îöåíêó
	//Timer t;
	for (auto& pos : movesVec) {
		TryMove(pos, Col, enemyCol);
	}
}

/*void BotV3::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//ïûòàåìñÿ ñõîäèòü êàæäûì õîäîì, ÷òîáû ïðîèçâåñòè îöåíêó

	std::vector<std::future<void>> vecAsy(movesVec.size());


	for (int i = 0; i < movesVec.size(); i++) {
		vecAsy[i] = std::async(std::launch::async, [&]() { TryMove(movesVec.at(i), Col, enemyCol); });
		vecAsy[i].get();
	}
	
	vecAsy.clear();
}*/



//ôóíêöèÿ âîçâðàùåíèÿ ñàìîãî ðåéòèíãîâîãî õîäà
BotV3::PosTurn BotV3::ReturnMove() {

	//çàïîëíÿåì âåêòîðà ìîèõ õîäîâ è âîçìîæíûõ õîäîâ ñîïåðíèêà
	FillPosTurns(PosBoard, PosMoves, color, enemycolor);
	FillPosTurns(PosBoard, PosMovesEnemy, enemycolor, color);

	//ïðîçèâîäèì îöåíêó âñåì íàøèì âîçìîæíûì õîäàì
	TryMoveAllPoss(PosMoves, color, enemycolor);

	std::vector<PosTurn> moves;
	PosTurn max;
	max.relevance = INT_MIN;

	//âû÷èñëÿåì ìàêñèìàëüíûé õîä ïî ðåéòèíãó
	for (auto& move : PosMoves) {
		if (move.relevance > max.relevance) {
			max = move;
		}
	}


	//åñëè åñòü îäèíàêîâûå õîäû ïî ðåéòèíãó, çàïîëíÿåì èìè âåêòîð õîäîâ
	for (auto& move : PosMoves) {
		if (move.relevance == max.relevance) {
			moves.push_back(move);
		}
	}
	moves.push_back(max);

	srand(time(NULL));
	//åñëè åñòü ðàâíûå ïî öåííîñòè õîäû, âîçâðàùàåì ñëó÷àéíûé èç íèõ
	if (moves.size() > 1) {
		int randomINDEX = rand() % (moves.size() - 1);
		return moves[randomINDEX];
	}//åñëè íåò, òî ñàìûé ïåðâûé
	else if (moves.size() == 1) {
		return moves[0];
	}
}



