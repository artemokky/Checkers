# Checkers
Game Checkers with bot

![image](https://user-images.githubusercontent.com/72733482/172914094-69d14f13-aac4-4357-b7ae-406b828a7e9b.png)

![image](https://user-images.githubusercontent.com/72733482/172914271-8709bb34-1949-4aa3-85f5-1ddd51482497.png)

Игра в шашки против трех ботов или живого игрока на одном компьютере

Игровое поле само показывает возможные ходы, при выборе шашки, которой можно сходить


В игре присутствуют три бота, разных по сложности и тактике
Разновидности ботов:
1. Легкий бот - всегда играет по одной простой тактике
2. Случайный бот - всегда выбирает случайный ход
3. Умный бот - просчитывает возможные ходы и ходы соперника



Приблизительный алгоритм умного бота:
1. Находит все возможные ходы в соответствии с правилами
2. На копии игрового поля выполняет эти ходы, оценивая их результативность
3. Для каждого шага на копии игрового поля выполняются все возможные ходы соперника по очереди, оценивается результативность этих ходов, делаются поправки в рейтинг нашего шага
4. Оцениваются все возможные появившиеся ходы после "ходов" соперника, рейтинг ходов бота изменяется с пониженным коэффициентом
5. Среди всех ходов выбирается ход с максимальным рейтингом
6. Выполняется ход с максимальным рейтингом



Рейтинг возможных ситупаций на игровом поле, которым пользуется умный бот:
1. EAT = 300           (Может съесть)
2. EAT_MORE = 600      (Может съесть больше 1 шашки)
3. EAT_KING = 1000     (Может съесть дамку)
4. SAVE = 500          (Может спасти от съедения)
5. BLOCK = 40          (Может заблокировать ход сопернику)
6. BECOME_KING = 3000  (Может стать дамкой)
7. DIE = -400          (Может быть съеден)
8. DIE_MORE = -1000    (Могут быть съедены и другие шашки)
9. CAN_GO = 2          (Может сходить)
