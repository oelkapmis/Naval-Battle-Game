#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<allegro5\allegro.h>
#include<allegro5\allegro_primitives.h>
#include<allegro5\allegro_native_dialog.h>
#include<allegro5\allegro_image.h>
#include<allegro5\allegro_font.h>
#include<allegro5\allegro_ttf.h>

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *font36 = NULL;
ALLEGRO_FONT *font24 = NULL;
ALLEGRO_BITMAP *PlayerBlue = NULL;
ALLEGRO_BITMAP *PlayerSquare = NULL;
ALLEGRO_BITMAP *Naval2h = NULL;
ALLEGRO_BITMAP *Naval3h = NULL;
ALLEGRO_BITMAP *Naval4h = NULL;
ALLEGRO_BITMAP *Naval5h = NULL;
ALLEGRO_BITMAP *Naval2v = NULL;
ALLEGRO_BITMAP *Naval3v = NULL;
ALLEGRO_BITMAP *Naval4v = NULL;
ALLEGRO_BITMAP *Naval5v = NULL;
ALLEGRO_BITMAP *Rotate = NULL;
ALLEGRO_BITMAP *Place = NULL;
ALLEGRO_BITMAP *Hitimage = NULL;
ALLEGRO_BITMAP *Missimage = NULL;
ALLEGRO_BITMAP *Aim = NULL;

const int sqrlen = 40;//Haritalardaki bir birim karenin uzunluðu
const int Width = 920;//Ekranýmýzýn x uzunluðu
const int Height = 520;//Ekranýmýzýn y uzunluðu


void Drawing();//Ekrana çizilmesi gereken þeyleri yazdýran fonksiyon
void LoadBitmaps();//Bitmap'leri programa yükleyen fonksiyon
void InitValues();//Bitmap'lerin baþlangýç deðerlerini atayan fonksiyon
void Destroy();//Program kapanmadan önce çaðýracaðýmýz ve tüm verileri yok edecek olan fonksiyon

void Animation(int t); //Yerleþtirilecek olan geminin yanýp sönme animasyonunu yapar. 't' : type of naval
bool IsClickedRotate();//Rotate butonuna týklanýlýp týklanýlmadýðýný kontrol eder
bool IsClickedPlace(); //Place butonuna týklanýlýp týklanýlmadýðýný kontrol eder
bool IsClickedMap(int x, int y);//x ve y, bakmasýný istediðimiz haritanýn baþlangýç koordinatlarý
bool IsInMap(int playername); //Yerleþtirilecek olan geminin haritaya sýðýp sýðmadýðýný kontrol eder
bool IsInDeadZone(int playername); //Gemiyi yerleþtireceðimiz alanýn boþ mu dolu mu olduðunu kontrol eder
void AddToDeadZone(int playername); // Yerleþtirdiðimiz alaný dolu olarak tanýmlar
void SlideIntoMap(int playername); //Yerleþtirilecek olan geminin dýþarýda kalan kýsmýný haritanýn içine doðru kaydýrýr
int FindSqrx(int coordinate);//Aldýðý x koordinat deðerinin haritadaki kaçýncý kutuya denk geldiðini söyler
int FindSqry(int coordinate);//Aldýðý y koordinat deðerinin haritadaki kaçýncý kutuya denk geldiðini söyler
void Fire(int playername);

bool IsThereANaval(int playername, int row, int col);
bool IsAllNavalsDestroyed(int playername);

void CreateComputerNavals();
void AddToDeadZone2(int which);
void SetProbabilityMap();
void CalculatePMapFor(int navalnum);

typedef struct {
	bool map[10][10];
	int x;
	int y;
	int width;
	int height;

}MAPS;//Harita bilgilerini tutar
MAPS *maps;
struct USERNAVALS {
	int x; int y;
	int direction;
	int len;
	bool IsPlaced;//Pregame'de animasyon yaparken kullanilir yerlesmediyse yanip soner
	bool IsDestroyed;//Gemi, uzunlugu kadar vurulduysa destroyed olur
	int point;//Geminin kac defa vuruldugunu tutar her vurulusunda 1 artar
};//Kullanýcýnýn gemi bilgilerini tutar
USERNAVALS *usernavals;
struct COMPUTERNAVALS {
	int x; int y;
	int direction;
	int len;
	bool IsPlaced;
	bool IsDestroyed;
	int point;
};//Bilgisayarýn gemi bilgilerini tutar
COMPUTERNAVALS *computernavals;

typedef struct {
	int x;
	int y;
	int width;
	int height;
}ROTATEBUTTON;//Rotate buton bilgilerini tutar
ROTATEBUTTON rotate;
typedef struct {
	int x;
	int y;
	int width;
	int height;
}PLACEBUTTON;//Place buton bilgilerini tutar
PLACEBUTTON place;

enum STATE { Pregame, Game, End };//Oyunumuzun kýsýmlarý, Oyun öncesi, Oynama Esnasý, Bitiþ
enum PLAYERS { User, Computer };//Oyuncularýn adlarý Kullanýcý ve Bilgisayar
enum DIRECTIONS { horizontal, vertical };//Gemilerin yönünü belirten deðerler Yatay, Dikey
enum NAVALNAMES { two, three1, three2, four, five }; //Gemimizin adý 2'li gemi, 4'lü gemi anlamýnda vs.(3'lüden 2 tane var)
enum MAPINFORMATIONS { NotAimed, MissAim, HitAim };
enum LASTFIRE { Miss, Hit };




//---------------------------------------------
//----------PREGAME VARIABLES------------------
int AnimationFrame = 0;//Animasyonumuzun baþlangýcýndan sonuna kadar geçen her bir görüntünün sayýsý
bool active;//Animasyonumuzu kapat ac yaparken kontrol etmeye yariyor
bool textactive = false;//Kullanýcý hatalý yerleþtirme yaparsa bu bool deðerine göre ekrana uyarý yazýsý yazdýracaðýz
int t = five;//type of the usernaval in animation, number 4 is enum variable of the naval
int k = five;//type of the computernaval in pregame, number 4 is enum variable of the naval
bool DeadZone[10][10];//Bu dizide ölü olan yerler için true deðerini tutacaðýz. Bir nevi haritanýn neresinin dolu olduðunu gösteriyor
//---------------------------------------------


//---------------------------------------------
//----------GAME VARIABLES--------------------- Fire ile ilgili olanlar fonksiyonun icinde static olarak da tanimlanabilirdi
const int ComputerThinkingTime = 0; //Bilgisayarin atis yapmadan once kac saniye bekleyecegini gosterir 
bool DeadZone2[10][10]; //Gemi batirildiktan sonra komsu kareleri bu dizide oldurecegiz
int whichnaval;//Ates ettigimiz yerde eger bir gemi varsa o geminin tipini/indis numarasini bu degiskende tutacagiz
int CenterRow = -1, CenterCol = -1;
int AlternateRow = -1, AlternateCol = -1;
int NextRow, NextCol;
bool LastFire = Miss; int NextSquare = 1;
int UserMapInformation[10][10];//MapInformation dizileri haritalardaki herhangi bir noktanin
int ComputerMapInformation[10][10];//Daha once nisan alinip alinmadigini, alindiysa vurulup vurulmadigini gosterir
//Hic hedef alinmadiysa 0, hedef alinip iskalandiysa 1, hedef alinip vurulduysa 2
int ProbabilityMap[10][10];

//---------------------------------------------


int state;//State deðeri oyunun hangi kýsmýnda olduðumuzu tutacak. Pregame ve Game kýsýmlarýný bu state'e atayacaðýz.
int mouse_x, mouse_y;//Faremizin koordinatlarýný tutacaðýz
int main(void)
{
	maps = (MAPS*)calloc(2, sizeof(MAPS));
	usernavals = (USERNAVALS*)calloc(5, sizeof(USERNAVALS));
	computernavals = (COMPUTERNAVALS*)calloc(5, sizeof(COMPUTERNAVALS)); //Burada struct'larýmýz için bellekte yer ayýrýyoruz

	srand(time(NULL)); //Rastgele sayý üretmemizi saðlayacak olan fonksiyon
	al_init();
	al_init_image_addon();
	al_init_primitives_addon();
	al_install_mouse();
	al_init_font_addon();
	al_init_ttf_addon();
	font36 = al_load_font("arial.ttf", 36, NULL); //Allegronun ilk atama fonksiyonlarý. Kullanacaðýmýz þeyleri programa tanýtýyoruz
	font24 = al_load_font("arial.ttf", 24, NULL);

	display = al_create_display(Width, Height); //Ekraný oluþturuyoruz
	event_queue = al_create_event_queue(); //Olay sýrasýný takip edeceðimiz veri
	timer = al_create_timer(1.0 / 4);//Saniyede 4 defa time loop yapar. yani her ceyrek saniyede bir timer aktif olur, animasyon için kullanacaðýz
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	LoadBitmaps();
	InitValues();

	while (1)
	{
		Drawing();
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			break;
		else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
		{
			mouse_x = ev.mouse.x;
			mouse_y = ev.mouse.y;

		}
		else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{

			if (state == Pregame) {

				if (IsClickedMap(maps[User].x, maps[User].y))
				{
					usernavals[t].x = maps[User].x + (sqrlen*FindSqrx(mouse_x));
					usernavals[t].y = maps[User].y + (sqrlen*FindSqry(mouse_y));
					if (!IsInMap(User)) SlideIntoMap(User);
				}
				else if (IsClickedRotate())
				{
					usernavals[t].direction = !(usernavals[t].direction);
					if (!IsInMap(User))  SlideIntoMap(User);
				}
				else if (IsClickedPlace())
				{
					if (!IsInDeadZone(User))
					{
						textactive = false;
						AddToDeadZone(User);
						usernavals[t].IsPlaced = true;


						if (t == two)
							state = Game;

						//Son gemi de yerlesince pregame yerine artik game bolumune geciyoruz
						//User'in bir gemisi aciga ciktiginda etrafi deadzone olacak ve bilgisayar oraya atis yapmayacak
						t--;
					}
					else textactive = true;
				}
			}
			else if (state == Game) {

				if (mouse_x >= maps[Computer].x && mouse_x <= maps[Computer].x + maps[Computer].width)//bilgisayarin ekranina tiklandiysa hedef alindiysa
					if (mouse_y >= maps[Computer].y && mouse_y <= maps[Computer].y + maps[Computer].height)
					{
						if (ComputerMapInformation[FindSqry(mouse_y)][FindSqrx(mouse_x)] == NotAimed) //Verilen koordinatlara daha once atis yapilmamissa
						{
							Fire(User);//Her sey atisa uygun olunca atisimizi yapiyoruz
							Drawing();
							if (!IsAllNavalsDestroyed(Computer)) {

								al_rest(ComputerThinkingTime);
								while (al_get_next_event(event_queue, &ev)) //Bilgisayar bekleme suresine girince eger kullanici event_queue'ya ekleme yaparsa
								{//Bizim bunu bosaltmamiz gerekir. Mouse'u hareket ettirirse mouse pozisyonunu guncellesin istiyorum bu nedenle
								 //event queue bosalana kadar mouse hareketlerini guncelliyorum diger hareketler icin hicbir eylem yapmiyorum
									if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
									{
										mouse_x = ev.mouse.x;
										mouse_y = ev.mouse.y;
									}
								}
								Fire(Computer);
							}
							else state = End;

							if (IsAllNavalsDestroyed(User)) state = End;

						}
					}

			}
			else if (state == End)
			{
				if (mouse_y > Height * 7 / 8)
				{
					InitValues();
				}
			}
		}
		else if (ev.type == ALLEGRO_EVENT_TIMER)
		{
			Animation(t);
			Drawing();
			if (state == Game) { CreateComputerNavals(); al_stop_timer(timer); }
		}

	}

	Destroy();
	return 0;
}

void LoadBitmaps() {

	PlayerBlue = al_load_bitmap("PlayerBlue.png");
	PlayerSquare = al_load_bitmap("PlayerSquare.png");
	al_convert_mask_to_alpha(PlayerSquare, al_map_rgb(255, 255, 255));
	Naval2h = al_load_bitmap("Naval2h.png");
	al_convert_mask_to_alpha(Naval2h, al_map_rgb(255, 255, 255));
	Naval3h = al_load_bitmap("Naval3h.png");
	al_convert_mask_to_alpha(Naval3h, al_map_rgb(255, 255, 255));
	Naval4h = al_load_bitmap("Naval4h.png");
	al_convert_mask_to_alpha(Naval4h, al_map_rgb(255, 255, 255));
	Naval5h = al_load_bitmap("Naval5h.png");
	al_convert_mask_to_alpha(Naval5h, al_map_rgb(255, 255, 255));
	Naval2v = al_load_bitmap("Naval2v.png");
	al_convert_mask_to_alpha(Naval2v, al_map_rgb(255, 255, 255));
	Naval3v = al_load_bitmap("Naval3v.png");
	al_convert_mask_to_alpha(Naval3v, al_map_rgb(255, 255, 255));
	Naval4v = al_load_bitmap("Naval4v.png");
	al_convert_mask_to_alpha(Naval4v, al_map_rgb(255, 255, 255));
	Naval5v = al_load_bitmap("Naval5v.png");
	al_convert_mask_to_alpha(Naval5v, al_map_rgb(255, 255, 255));
	Rotate = al_load_bitmap("Rotate.png");
	al_convert_mask_to_alpha(Rotate, al_map_rgb(255, 255, 255));
	Place = al_load_bitmap("Place.png");
	al_convert_mask_to_alpha(Place, al_map_rgb(255, 255, 255));
	Hitimage = al_load_bitmap("Hit.png");
	al_convert_mask_to_alpha(Hitimage, al_map_rgb(255, 255, 255));
	Missimage = al_load_bitmap("Miss.png");
	al_convert_mask_to_alpha(Missimage, al_map_rgb(255, 255, 255));
	Aim = al_load_bitmap("Aim.png");
	al_convert_mask_to_alpha(Aim, al_map_rgb(255, 255, 255));


}
void InitValues() {

	state = Pregame;
	al_start_timer(timer);
	t = five;
	k = five;
	textactive = false;
	AnimationFrame = 0;
	CenterRow = -1, CenterCol = -1;
	int AlternateRow = -1, AlternateCol = -1;
	LastFire = Miss;
	NextSquare = 1;


	rotate.x = 140;
	rotate.y = 130;
	rotate.width = al_get_bitmap_width(Rotate);
	rotate.height = al_get_bitmap_height(Rotate);

	place.x = 140;
	place.y = 290;
	place.width = al_get_bitmap_width(Place);
	place.height = al_get_bitmap_height(Place);

	maps[User].width = al_get_bitmap_width(PlayerSquare);
	maps[User].height = al_get_bitmap_height(PlayerSquare);
	maps[User].x = 480; maps[User].y = 40;

	maps[Computer].width = al_get_bitmap_width(PlayerSquare);
	maps[Computer].height = al_get_bitmap_height(PlayerSquare);
	maps[Computer].x = 40; maps[Computer].y = 40;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
		{
			maps[User].map[i][j] = false;
			maps[Computer].map[i][j] = false;
			DeadZone[i][j] = false;
			DeadZone2[i][j] = false;
			UserMapInformation[i][j] = 0;
			ComputerMapInformation[i][j] = 0;
		}
	for (int i = 0; i < 5; i++)
	{
		usernavals[i].direction = horizontal;
	}
	for (int i = 0; i < 5; i++)//Bilgisayarin gemilerinin yonunu rastgele belirliyoruz
	{
		if (rand() % 2 == 0) //Rastgele bir sayi uretiyoruz ve
			computernavals[i].direction = horizontal;//sayi cift ise gemi yatay, 
		else
			computernavals[i].direction = vertical;//sayi tek ise gemi dik oluyor
	}
	usernavals[two].len = 2;
	usernavals[three1].len = 3;
	usernavals[three2].len = 3;
	usernavals[four].len = 4;
	usernavals[five].len = 5;
	computernavals[two].len = 2;
	computernavals[three1].len = 3;
	computernavals[three2].len = 3;
	computernavals[four].len = 4;
	computernavals[five].len = 5;

	for (int i = 0; i < 5; i++)
	{
		usernavals[i].x = maps[User].x;
		usernavals[i].y = maps[User].y;
		usernavals[i].IsPlaced = false;
		computernavals[i].IsPlaced = false;
		usernavals[i].IsDestroyed = false;
		computernavals[i].IsDestroyed = false;
		usernavals[i].point = 0;
		computernavals[i].point = 0;
	}
}
void Destroy() {
	free(maps);
	free(usernavals);
	free(computernavals);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_timer(timer);
	al_destroy_font(font36);
	al_destroy_bitmap(PlayerBlue);
	al_destroy_bitmap(PlayerSquare);
	al_destroy_bitmap(Naval2h);
	al_destroy_bitmap(Naval3h);
	al_destroy_bitmap(Naval4h);
	al_destroy_bitmap(Naval5h);
	al_destroy_bitmap(Naval2v);
	al_destroy_bitmap(Naval3v);
	al_destroy_bitmap(Naval4v);
	al_destroy_bitmap(Naval5v);
	al_destroy_bitmap(Rotate);
	al_destroy_bitmap(Place);
	al_destroy_bitmap(Hitimage);
	al_destroy_bitmap(Missimage);
}

void Drawing() {


	if (state == Pregame) {

		al_draw_bitmap(PlayerBlue, maps[User].x, maps[User].y, NULL);

		if (usernavals[two].direction == horizontal && usernavals[two].IsPlaced)
			al_draw_bitmap(Naval2h, usernavals[two].x, usernavals[two].y, NULL);
		else if (usernavals[two].direction == vertical && usernavals[two].IsPlaced)
			al_draw_bitmap(Naval2v, usernavals[two].x, usernavals[two].y, NULL);
		if (usernavals[three1].direction == horizontal && usernavals[three1].IsPlaced)
			al_draw_bitmap(Naval3h, usernavals[three1].x, usernavals[three1].y, NULL);
		else if ((usernavals[three1].direction == vertical && usernavals[three1].IsPlaced))
			al_draw_bitmap(Naval3v, usernavals[three1].x, usernavals[three1].y, NULL);
		if (usernavals[three2].direction == horizontal && usernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3h, usernavals[three2].x, usernavals[three2].y, NULL);
		else if (usernavals[three2].direction == vertical && usernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3v, usernavals[three2].x, usernavals[three2].y, NULL);
		if (usernavals[four].direction == horizontal && usernavals[four].IsPlaced)
			al_draw_bitmap(Naval4h, usernavals[four].x, usernavals[four].y, NULL);
		else if (usernavals[four].direction == vertical && usernavals[four].IsPlaced)
			al_draw_bitmap(Naval4v, usernavals[four].x, usernavals[four].y, NULL);
		if (usernavals[five].direction == horizontal && usernavals[five].IsPlaced)
			al_draw_bitmap(Naval5h, usernavals[five].x, usernavals[five].y, NULL);
		else if (usernavals[five].direction == vertical && usernavals[five].IsPlaced)
			al_draw_bitmap(Naval5v, usernavals[five].x, usernavals[five].y, NULL);


		if (active)
		{
			switch (t)
			{
			case two:	if (usernavals[two].direction == horizontal)al_draw_bitmap(Naval2h, usernavals[two].x, usernavals[two].y, NULL);
						else al_draw_bitmap(Naval2v, usernavals[two].x, usernavals[two].y, NULL); break;
			case three1:	if (usernavals[three1].direction == horizontal)al_draw_bitmap(Naval3h, usernavals[three1].x, usernavals[three1].y, NULL);
							else al_draw_bitmap(Naval3v, usernavals[three1].x, usernavals[three1].y, NULL); break;
			case three2:	if (usernavals[three2].direction == horizontal)al_draw_bitmap(Naval3h, usernavals[three2].x, usernavals[three2].y, NULL);
							else al_draw_bitmap(Naval3v, usernavals[three2].x, usernavals[three2].y, NULL); break;
			case four:	if (usernavals[four].direction == horizontal)al_draw_bitmap(Naval4h, usernavals[four].x, usernavals[four].y, NULL);
						else al_draw_bitmap(Naval4v, usernavals[four].x, usernavals[four].y, NULL); break;
			case five:	if (usernavals[five].direction == horizontal)al_draw_bitmap(Naval5h, usernavals[five].x, usernavals[five].y, NULL);
						else al_draw_bitmap(Naval5v, usernavals[five].x, usernavals[five].y, NULL); break;
			}
		}

		al_draw_bitmap(PlayerSquare, maps[User].x, maps[User].y, NULL);
		al_draw_bitmap(Rotate, rotate.x, rotate.y, NULL);
		al_draw_bitmap(Place, place.x, place.y, NULL);

		if (textactive)
			al_draw_text(font24, al_map_rgb(255, 255, 255), Width * 3 / 5, Height * 7 / 8, ALLEGRO_ALIGN_CENTRE, "Ships must not overlap or occupy squares next to other ships!");

		//al_draw_textf(font36, al_map_rgb(255, 0, 255), Width / 2, Height * 7 / 8 + 7, ALLEGRO_ALIGN_CENTER, "Second is : %i", AnimationFrame);
	}
	else if (state == Game) {
		al_draw_bitmap(PlayerBlue, maps[User].x, maps[User].y, NULL);
		al_draw_bitmap(PlayerBlue, maps[Computer].x, maps[Computer].y, NULL);

		if (usernavals[two].direction == horizontal)
			al_draw_bitmap(Naval2h, usernavals[two].x, usernavals[two].y, NULL);
		else if (usernavals[two].direction == vertical)
			al_draw_bitmap(Naval2v, usernavals[two].x, usernavals[two].y, NULL);
		if (usernavals[three1].direction == horizontal)
			al_draw_bitmap(Naval3h, usernavals[three1].x, usernavals[three1].y, NULL);
		else if ((usernavals[three1].direction == vertical))
			al_draw_bitmap(Naval3v, usernavals[three1].x, usernavals[three1].y, NULL);
		if (usernavals[three2].direction == horizontal)
			al_draw_bitmap(Naval3h, usernavals[three2].x, usernavals[three2].y, NULL);
		else if (usernavals[three2].direction == vertical)
			al_draw_bitmap(Naval3v, usernavals[three2].x, usernavals[three2].y, NULL);
		if (usernavals[four].direction == horizontal)
			al_draw_bitmap(Naval4h, usernavals[four].x, usernavals[four].y, NULL);
		else if (usernavals[four].direction == vertical)
			al_draw_bitmap(Naval4v, usernavals[four].x, usernavals[four].y, NULL);
		if (usernavals[five].direction == horizontal)
			al_draw_bitmap(Naval5h, usernavals[five].x, usernavals[five].y, NULL);
		else if (usernavals[five].direction == vertical)
			al_draw_bitmap(Naval5v, usernavals[five].x, usernavals[five].y, NULL);
		/*
		if (computernavals[two].direction == horizontal)
			al_draw_bitmap(Naval2h, computernavals[two].x, computernavals[two].y, NULL);
		else if (computernavals[two].direction == vertical)
			al_draw_bitmap(Naval2v, computernavals[two].x, computernavals[two].y, NULL);
		if (computernavals[three1].direction == horizontal)
			al_draw_bitmap(Naval3h, computernavals[three1].x, computernavals[three1].y, NULL);
		else if ((computernavals[three1].direction == vertical))
			al_draw_bitmap(Naval3v, computernavals[three1].x, computernavals[three1].y, NULL);
		if (computernavals[three2].direction == horizontal)
			al_draw_bitmap(Naval3h, computernavals[three2].x, computernavals[three2].y, NULL);
		else if (computernavals[three2].direction == vertical)
			al_draw_bitmap(Naval3v, computernavals[three2].x, computernavals[three2].y, NULL);
		if (computernavals[four].direction == horizontal)
			al_draw_bitmap(Naval4h, computernavals[four].x, computernavals[four].y, NULL);
		else if (computernavals[four].direction == vertical)
			al_draw_bitmap(Naval4v, computernavals[four].x, computernavals[four].y, NULL);
		if (computernavals[five].direction == horizontal)
			al_draw_bitmap(Naval5h, computernavals[five].x, computernavals[five].y, NULL);
		else if (computernavals[five].direction == vertical)
			al_draw_bitmap(Naval5v, computernavals[five].x, computernavals[five].y, NULL);
			*/

		if (computernavals[two].direction == horizontal && computernavals[two].point == computernavals[two].len)
			al_draw_bitmap(Naval2h, computernavals[two].x, computernavals[two].y, NULL);
		else if (computernavals[two].direction == vertical && computernavals[two].point == computernavals[two].len)
			al_draw_bitmap(Naval2v, computernavals[two].x, computernavals[two].y, NULL);
		if (computernavals[three1].direction == horizontal && computernavals[three1].point == computernavals[three1].len)
			al_draw_bitmap(Naval3h, computernavals[three1].x, computernavals[three1].y, NULL);
		else if ((computernavals[three1].direction == vertical && computernavals[three1].point == computernavals[three1].len))
			al_draw_bitmap(Naval3v, computernavals[three1].x, computernavals[three1].y, NULL);
		if (computernavals[three2].direction == horizontal && computernavals[three2].point == computernavals[three2].len)
			al_draw_bitmap(Naval3h, computernavals[three2].x, computernavals[three2].y, NULL);
		else if (computernavals[three2].direction == vertical && computernavals[three2].point == computernavals[three2].len)
			al_draw_bitmap(Naval3v, computernavals[three2].x, computernavals[three2].y, NULL);
		if (computernavals[four].direction == horizontal && computernavals[four].point == computernavals[four].len)
			al_draw_bitmap(Naval4h, computernavals[four].x, computernavals[four].y, NULL);
		else if (computernavals[four].direction == vertical && computernavals[four].point == computernavals[four].len)
			al_draw_bitmap(Naval4v, computernavals[four].x, computernavals[four].y, NULL);
		if (computernavals[five].direction == horizontal && computernavals[five].point == computernavals[five].len)
			al_draw_bitmap(Naval5h, computernavals[five].x, computernavals[five].y, NULL);
		else if (computernavals[five].direction == vertical && computernavals[five].point == computernavals[five].len)
			al_draw_bitmap(Naval5v, computernavals[five].x, computernavals[five].y, NULL);



		al_draw_bitmap(PlayerSquare, maps[User].x, maps[User].y, NULL);
		al_draw_bitmap(PlayerSquare, maps[Computer].x, maps[Computer].y, NULL);

		/**/for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
			{
				if (UserMapInformation[i][j] == MissAim)
				{
					al_draw_bitmap(Missimage, maps[User].x + (j*sqrlen), maps[User].y + (i*sqrlen), NULL);
					// al_draw_textf(font36, al_map_rgb(255, 255, 255), maps[User].x+ maps[User].width / 2, Height * 7 / 8, ALLEGRO_ALIGN_CENTER, "Computer aim to [%d][%d] and miss", i+1, j+1);
				}
				else if (UserMapInformation[i][j] == HitAim) { al_draw_bitmap(Hitimage, maps[User].x + (j*sqrlen), maps[User].y + (i*sqrlen), NULL); }
				if (ComputerMapInformation[i][j] == MissAim) { al_draw_bitmap(Missimage, maps[Computer].x + (j*sqrlen), maps[Computer].y + (i*sqrlen), NULL); }
				else if (ComputerMapInformation[i][j] == HitAim) { al_draw_bitmap(Hitimage, maps[Computer].x + (j*sqrlen), maps[Computer].y + (i*sqrlen), NULL); }
			}

	}
	else if (state == End) {


		al_draw_bitmap(PlayerBlue, maps[User].x, maps[User].y, NULL);
		al_draw_bitmap(PlayerBlue, maps[Computer].x, maps[Computer].y, NULL);

		if (usernavals[two].direction == horizontal && usernavals[two].IsPlaced)
			al_draw_bitmap(Naval2h, usernavals[two].x, usernavals[two].y, NULL);
		else if (usernavals[two].direction == vertical && usernavals[two].IsPlaced)
			al_draw_bitmap(Naval2v, usernavals[two].x, usernavals[two].y, NULL);
		if (usernavals[three1].direction == horizontal && usernavals[three1].IsPlaced)
			al_draw_bitmap(Naval3h, usernavals[three1].x, usernavals[three1].y, NULL);
		else if ((usernavals[three1].direction == vertical && usernavals[three1].IsPlaced))
			al_draw_bitmap(Naval3v, usernavals[three1].x, usernavals[three1].y, NULL);
		if (usernavals[three2].direction == horizontal && usernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3h, usernavals[three2].x, usernavals[three2].y, NULL);
		else if (usernavals[three2].direction == vertical && usernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3v, usernavals[three2].x, usernavals[three2].y, NULL);
		if (usernavals[four].direction == horizontal && usernavals[four].IsPlaced)
			al_draw_bitmap(Naval4h, usernavals[four].x, usernavals[four].y, NULL);
		else if (usernavals[four].direction == vertical && usernavals[four].IsPlaced)
			al_draw_bitmap(Naval4v, usernavals[four].x, usernavals[four].y, NULL);
		if (usernavals[five].direction == horizontal && usernavals[five].IsPlaced)
			al_draw_bitmap(Naval5h, usernavals[five].x, usernavals[five].y, NULL);
		else if (usernavals[five].direction == vertical && usernavals[five].IsPlaced)
			al_draw_bitmap(Naval5v, usernavals[five].x, usernavals[five].y, NULL);

		if (computernavals[two].direction == horizontal && computernavals[two].IsPlaced)
			al_draw_bitmap(Naval2h, computernavals[two].x, computernavals[two].y, NULL);
		else if (computernavals[two].direction == vertical && computernavals[two].IsPlaced)
			al_draw_bitmap(Naval2v, computernavals[two].x, computernavals[two].y, NULL);
		if (computernavals[three1].direction == horizontal && computernavals[three1].IsPlaced)
			al_draw_bitmap(Naval3h, computernavals[three1].x, computernavals[three1].y, NULL);
		else if ((computernavals[three1].direction == vertical && computernavals[three1].IsPlaced))
			al_draw_bitmap(Naval3v, computernavals[three1].x, computernavals[three1].y, NULL);
		if (computernavals[three2].direction == horizontal && computernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3h, computernavals[three2].x, computernavals[three2].y, NULL);
		else if (computernavals[three2].direction == vertical && computernavals[three2].IsPlaced)
			al_draw_bitmap(Naval3v, computernavals[three2].x, computernavals[three2].y, NULL);
		if (computernavals[four].direction == horizontal && computernavals[four].IsPlaced)
			al_draw_bitmap(Naval4h, computernavals[four].x, computernavals[four].y, NULL);
		else if (computernavals[four].direction == vertical && computernavals[four].IsPlaced)
			al_draw_bitmap(Naval4v, computernavals[four].x, computernavals[four].y, NULL);
		if (computernavals[five].direction == horizontal && computernavals[five].IsPlaced)
			al_draw_bitmap(Naval5h, computernavals[five].x, computernavals[five].y, NULL);
		else if (computernavals[five].direction == vertical && computernavals[five].IsPlaced)
			al_draw_bitmap(Naval5v, computernavals[five].x, computernavals[five].y, NULL);

		/*
		if (computernavals[two].direction == horizontal && computernavals[two].point == computernavals[two].len)
			al_draw_bitmap(Naval2h, computernavals[two].x, computernavals[two].y, NULL);
		else if (computernavals[two].direction == vertical && computernavals[two].point == computernavals[two].len)
			al_draw_bitmap(Naval2v, computernavals[two].x, computernavals[two].y, NULL);
		if (computernavals[three1].direction == horizontal && computernavals[three1].point == computernavals[three1].len)
			al_draw_bitmap(Naval3h, computernavals[three1].x, computernavals[three1].y, NULL);
		else if ((computernavals[three1].direction == vertical && computernavals[three1].point == computernavals[three1].len))
			al_draw_bitmap(Naval3v, computernavals[three1].x, computernavals[three1].y, NULL);
		if (computernavals[three2].direction == horizontal && computernavals[three2].point == computernavals[three2].len)
			al_draw_bitmap(Naval3h, computernavals[three2].x, computernavals[three2].y, NULL);
		else if (computernavals[three2].direction == vertical && computernavals[three2].point == computernavals[three2].len)
			al_draw_bitmap(Naval3v, computernavals[three2].x, computernavals[three2].y, NULL);
		if (computernavals[four].direction == horizontal && computernavals[four].point == computernavals[four].len)
			al_draw_bitmap(Naval4h, computernavals[four].x, computernavals[four].y, NULL);
		else if (computernavals[four].direction == vertical && computernavals[four].point == computernavals[four].len)
			al_draw_bitmap(Naval4v, computernavals[four].x, computernavals[four].y, NULL);
		if (computernavals[five].direction == horizontal && computernavals[five].point == computernavals[five].len)
			al_draw_bitmap(Naval5h, computernavals[five].x, computernavals[five].y, NULL);
		else if (computernavals[five].direction == vertical && computernavals[five].point == computernavals[five].len)
			al_draw_bitmap(Naval5v, computernavals[five].x, computernavals[five].y, NULL);
		*/


		al_draw_bitmap(PlayerSquare, maps[User].x, maps[User].y, NULL);
		al_draw_bitmap(PlayerSquare, maps[Computer].x, maps[Computer].y, NULL);

		/**/for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
			{
				if (UserMapInformation[i][j] == MissAim) { al_draw_bitmap(Missimage, maps[User].x + (j*sqrlen), maps[User].y + (i*sqrlen), NULL); }
				else if (UserMapInformation[i][j] == HitAim) { al_draw_bitmap(Hitimage, maps[User].x + (j*sqrlen), maps[User].y + (i*sqrlen), NULL); }
				if (ComputerMapInformation[i][j] == MissAim) { al_draw_bitmap(Missimage, maps[Computer].x + (j*sqrlen), maps[Computer].y + (i*sqrlen), NULL); }
				else if (ComputerMapInformation[i][j] == HitAim) { al_draw_bitmap(Hitimage, maps[Computer].x + (j*sqrlen), maps[Computer].y + (i*sqrlen), NULL); }
			}


		if (IsAllNavalsDestroyed(Computer))
		{
			al_draw_textf(font36, al_map_rgb(255, 0, 255), Width / 2, Height * 7 / 8 + 7, ALLEGRO_ALIGN_CENTER, "User Won! Click Here To Restart");
		}
		else if (IsAllNavalsDestroyed(User))
		{
			al_draw_textf(font36, al_map_rgb(255, 0, 255), Width / 2, Height * 7 / 8 + 7, ALLEGRO_ALIGN_CENTER, "Computer Won! Click Here To Restart");
		}
		al_draw_rounded_rectangle(20, Height * 7 / 8, Width - 20, Height - 10, 5, 5, al_map_rgb(255, 0, 255), 3);
	}


	al_flip_display();
	al_clear_to_color(al_map_rgb(0, 0, 0));
}
bool IsClickedRotate() {
	if (mouse_x >= rotate.x && mouse_x <= (rotate.x + rotate.width))
	{
		if (mouse_y >= rotate.y && mouse_y <= (rotate.y + rotate.height))
		{
			return true;
		}
		else return false;
	}
	else return false;
}
bool IsClickedPlace() {
	if (mouse_x >= place.x && mouse_x <= (place.x + place.width))
	{
		if (mouse_y >= place.y && mouse_y <= (place.y + place.height))
		{
			return true;
		}
		else return false;
	}
	else return false;
}
bool IsClickedMap(int x, int y) {
	if (mouse_x >= x && mouse_x <= x + maps[User].width)
	{
		if (mouse_y >= y && mouse_y <= y + maps[User].height)
			return true;
		else
			return false;
	}
	else return false;
}
bool IsInMap(int playername) {

	if (playername == User) {
		if (usernavals[t].direction == horizontal)
		{
			if (usernavals[t].x + ((usernavals[t].len)*sqrlen) <= (maps[User].x + maps[User].width))
				return true;
			else return false;
		}
		else
		{
			if (usernavals[t].y + ((usernavals[t].len)*sqrlen) <= (maps[User].y + maps[User].height))
				return true;
			else return false;
		}
	}
	else
	{
		if (computernavals[k].direction == horizontal)
		{
			if (computernavals[k].x + ((computernavals[k].len)*sqrlen) <= (maps[Computer].x + maps[Computer].width))
				return true;
			else return false;
		}
		else
		{
			if (computernavals[k].y + ((computernavals[k].len)*sqrlen) <= (maps[Computer].y + maps[Computer].height))
				return true;
			else return false;
		}
	}
}
bool IsInDeadZone(int playername) {
	int row, col, length;

	if (playername == User)
	{
		if (usernavals[t].direction == horizontal)
		{
			for (length = 0; length < usernavals[t].len; length++)
			{
				col = FindSqrx(usernavals[t].x + (length*sqrlen));
				row = FindSqry(usernavals[t].y);
				if (DeadZone[row][col] == true)
					return true;
			}
			return false;
		}
		else
		{
			for (length = 0; length < usernavals[t].len; length++) {
				col = FindSqrx(usernavals[t].x);
				row = FindSqry(usernavals[t].y + (length*sqrlen));
				if (DeadZone[row][col] == true)
					return true;
			}
			return false;
		}
	}
	else
	{

		if (computernavals[k].direction == horizontal)
		{
			for (length = 0; length < computernavals[k].len; length++)
			{
				col = FindSqrx(computernavals[k].x + (length*sqrlen));
				row = FindSqry(computernavals[k].y);
				if (DeadZone[row][col] == true)
					return true;
			}
			return false;
		}
		else
		{
			for (length = 0; length < computernavals[k].len; length++) {
				col = FindSqrx(computernavals[k].x);
				row = FindSqry(computernavals[k].y + (length*sqrlen));
				if (DeadZone[row][col] == true)
					return true;
			}
			return false;
		}
	}

}
bool IsThereANaval(int playername, int row, int col)
{
	if (playername == User)
	{
		for (int i = 0; i < 5; i++)
		{
			if (usernavals[i].direction == horizontal)
			{
				if (maps[User].x + (col*sqrlen) >= usernavals[i].x && maps[User].x + (col*sqrlen) < usernavals[i].x + (usernavals[i].len*sqrlen))
					if (maps[User].y + (row*sqrlen) == usernavals[i].y)
					{
						whichnaval = i;
						return true;
					}
			}
			else
			{
				if (maps[User].x + (col*sqrlen) == usernavals[i].x)
					if (maps[User].y + (row*sqrlen) >= usernavals[i].y && maps[User].y + (row*sqrlen) < usernavals[i].y + (usernavals[i].len*sqrlen))
					{
						whichnaval = i;
						return true;
					}
			}
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			if (computernavals[i].direction == horizontal)
			{
				if (maps[Computer].x + (col*sqrlen) >= computernavals[i].x && maps[Computer].x + (col*sqrlen) < computernavals[i].x + (computernavals[i].len*sqrlen))
					if (maps[Computer].y + (row*sqrlen) == computernavals[i].y)
					{
						whichnaval = i;
						return true;
					}
			}
			else
			{
				if (maps[Computer].x + (col*sqrlen) == computernavals[i].x)
					if (maps[Computer].y + (row*sqrlen) >= computernavals[i].y && maps[Computer].y + (row*sqrlen) < computernavals[i].y + (computernavals[i].len*sqrlen))
					{
						whichnaval = i;
						return true;
					}
			}
		}
	}

	return false;
}
void AddToDeadZone(int playername) {

	int row, col, length;

	if (playername == User) {

		if (usernavals[t].direction == horizontal)
		{

			for (length = 0; length < usernavals[t].len; length++)
			{
				col = FindSqrx((usernavals[t].x) + (length*sqrlen));
				row = FindSqry(usernavals[t].y);

				if (row - 1 >= 0 && col - 1 >= 0)	DeadZone[row - 1][col - 1] = true;
				if (row - 1 >= 0)				DeadZone[row - 1][col] = true;
				if (row - 1 >= 0 && col + 1 <= 9)	DeadZone[row - 1][col + 1] = true;
				if (col - 1 >= 0)				DeadZone[row][col - 1] = true;
				DeadZone[row][col] = true;
				if (col + 1 <= 9)				DeadZone[row][col + 1] = true;
				if (row + 1 <= 9 && col - 1 >= 0)	DeadZone[row + 1][col - 1] = true;
				if (row + 1 <= 9)				DeadZone[row + 1][col] = true;
				if (row + 1 <= 9 && col + 1 <= 9)	DeadZone[row + 1][col + 1] = true;

			}
		}
		else
		{
			for (length = 0; length < usernavals[t].len; length++)
			{
				row = FindSqry(usernavals[t].y + (length*sqrlen));
				col = FindSqrx(usernavals[t].x);

				if (row - 1 >= 0 && col - 1 >= 0)	DeadZone[row - 1][col - 1] = true;
				if (row - 1 >= 0)				DeadZone[row - 1][col] = true;
				if (row - 1 >= 0 && col + 1 <= 9)	DeadZone[row - 1][col + 1] = true;
				if (col - 1 >= 0)				DeadZone[row][col - 1] = true;
				DeadZone[row][col] = true;
				if (col + 1 <= 9)				DeadZone[row][col + 1] = true;
				if (row + 1 <= 9 && col - 1 >= 0)	DeadZone[row + 1][col - 1] = true;
				if (row + 1 <= 9)				DeadZone[row + 1][col] = true;
				if (row + 1 <= 9 && col + 1 <= 9)	DeadZone[row + 1][col + 1] = true;

			}
		}
	}
	else if (playername == Computer)
	{

		if (computernavals[k].direction == horizontal)
		{

			for (length = 0; length < computernavals[k].len; length++)
			{
				col = FindSqrx((computernavals[k].x) + (length*sqrlen));
				row = FindSqry(computernavals[k].y);

				if (row - 1 >= 0 && col - 1 >= 0)	DeadZone[row - 1][col - 1] = true;
				if (row - 1 >= 0)				DeadZone[row - 1][col] = true;
				if (row - 1 >= 0 && col + 1 <= 9)	DeadZone[row - 1][col + 1] = true;
				if (col - 1 >= 0)				DeadZone[row][col - 1] = true;
				DeadZone[row][col] = true;
				if (col + 1 <= 9)				DeadZone[row][col + 1] = true;
				if (row + 1 <= 9 && col - 1 >= 0)	DeadZone[row + 1][col - 1] = true;
				if (row + 1 <= 9)				DeadZone[row + 1][col] = true;
				if (row + 1 <= 9 && col + 1 <= 9)	DeadZone[row + 1][col + 1] = true;

			}
		}
		else
		{
			for (length = 0; length < computernavals[k].len; length++)
			{
				row = FindSqry(computernavals[k].y + (length*sqrlen));
				col = FindSqrx(computernavals[k].x);

				if (row - 1 >= 0 && col - 1 >= 0)	DeadZone[row - 1][col - 1] = true;
				if (row - 1 >= 0)				DeadZone[row - 1][col] = true;
				if (row - 1 >= 0 && col + 1 <= 9)	DeadZone[row - 1][col + 1] = true;
				if (col - 1 >= 0)				DeadZone[row][col - 1] = true;
				DeadZone[row][col] = true;
				if (col + 1 <= 9)				DeadZone[row][col + 1] = true;
				if (row + 1 <= 9 && col - 1 >= 0)	DeadZone[row + 1][col - 1] = true;
				if (row + 1 <= 9)				DeadZone[row + 1][col] = true;
				if (row + 1 <= 9 && col + 1 <= 9)	DeadZone[row + 1][col + 1] = true;
			}
		}
	}
} //Pregame
void SlideIntoMap(int playername) {

	if (playername == User) {
		while (!IsInMap(User))
		{
			if (usernavals[t].direction == horizontal)
			{
				usernavals[t].x -= sqrlen;
			}
			else
			{
				usernavals[t].y -= sqrlen;
			}
		}
	}
	else
	{
		while (!IsInMap(Computer))
		{
			if (computernavals[k].direction == horizontal)
			{
				computernavals[k].x -= sqrlen;
			}
			else
			{
				computernavals[k].y -= sqrlen;
			}
		}
	}
}
int FindSqrx(int coordinate) {

	if (coordinate >= maps[User].x)
	{
		coordinate -= maps[User].x;
		return (coordinate - (coordinate % sqrlen)) / sqrlen;
	}
	else if (coordinate <= maps[Computer].x + maps[Computer].width)
	{
		coordinate -= maps[Computer].x;
		return (coordinate - (coordinate % sqrlen)) / sqrlen;
	}
	else exit(-1);
}
int FindSqry(int coordinate) {

	coordinate -= maps[User].y;//User ya da computer olmasi farketmez y'leri ayni cunku.
	return (coordinate - (coordinate % sqrlen)) / sqrlen;
}
void Animation(int t) {
	AnimationFrame++;


	if (AnimationFrame % 2 == 0) { active = true; t = two; }
	else { active = false; }

}
void Fire(int playername) {

	static int a = NULL, b = NULL;

	if (playername == User) {
		a = FindSqry(mouse_y);//Mouse koordinatlarini harita satir ve sutunlarina ceviriyoruz
		b = FindSqrx(mouse_x);
		if (IsThereANaval(Computer, a, b)) //Orada gemi varsa
		{
			ComputerMapInformation[a][b] = HitAim; //O noktanin bilgisini 2 yani vuruldu yapiyoruz
			computernavals[whichnaval].point++; //ve vurulan geminin point'i 1 artiyor
			if (computernavals[whichnaval].point == computernavals[whichnaval].len)
				computernavals[whichnaval].IsDestroyed = true;
		}
		else { ComputerMapInformation[a][b] = MissAim; }//Orada gemi yoksa o noktanin bilgisi 1 yani iskaladi oluyor
	}
	else
	{

		//1-)Tamamen Rastgele atis yapiyorsak
		if (CenterRow == -1)//Rastgele atis yaptigimiz kisim
		{
			while (1) {
				a = rand() % 10;//Rastgele atis yapilacak bir yer belirlemeyecegiz/ bunun yerine olasilik haritasindaki en olasi yeri sececegiz
				b = rand() % 10;//Direkt olarak a,b noktasina atis yapacagiz
				if (UserMapInformation[a][b] == NotAimed && !DeadZone2[a][b])//Bu islemlere gerek kalmayacak olasilik haritasi cikaracagiz ve en olasi noktayi a,b alacagiz
				{
					if (IsThereANaval(User, a, b))//Target moda gececegiz. a,b center olarak alinacak. 
					{
						UserMapInformation[a][b] = HitAim;
						usernavals[whichnaval].point++;
						CenterRow = a; CenterCol = b;
						LastFire = Hit;
						AlternateRow = -1; AlternateCol = -1;
						/*if (usernavals[whichnaval].point == usernavals[whichnaval].len)
						{
							usernavals[whichnaval].IsDestroyed = true;
							AddToDeadZone2(whichnaval);

							CenterRow = -1; CenterCol = -1;
							AlternateRow = -1; AlternateCol = -1;
							LastFire = Miss;
						}*/
					}
					else
					{
						UserMapInformation[a][b] = MissAim;
						CenterRow = -1; CenterCol = -1;
						AlternateRow = -1; AlternateCol = -1;
						LastFire = Miss;
					}

					break;
				}
			}
		}

		//2-)Ilk isabetimizi yaptiysak ve onun yanlarindan herhangi bir yone atis yapacaksak
		else if (CenterRow != -1 && LastFire == Hit && AlternateRow == -1)//Target mod acik, ama hangi yone gidecegimiz belli degil yani alternate row yok
		{
			while (1) {

				a = rand() % 10;//Burada rastgele nokta belirlemeyecegiz. bunun yerine center'a komsu olan karelerden en olasi olani sececegiz ve o noktaya atis yapacagiz
				b = rand() % 10;
				if (abs(CenterRow - a) + abs(CenterCol - b) == 1 && UserMapInformation[a][b] == NotAimed && a >= 0 && a <= 9 && b >= 0 && b <= 9 && !DeadZone2[a][b])
				{//Merkeze 1 birim uzaklikta olan ve daha once vurulmamis bir nokta sectik

					if (IsThereANaval(User, a, b))//vurduysak alternate row belirlendi demektir. gidecegimiz yon kesinlesti. nextrow degiskeni gidecegimiz yon vektoru
					{
						AlternateRow = a;
						AlternateCol = b;

						UserMapInformation[a][b] = HitAim;
						usernavals[whichnaval].point++;
						LastFire = Hit;

						if (usernavals[whichnaval].point == usernavals[whichnaval].len)
						{
							usernavals[whichnaval].IsDestroyed = true;
							AddToDeadZone2(whichnaval);

							CenterRow = -1; CenterCol = -1;
							AlternateRow = -1; AlternateCol = -1;
							LastFire = Miss;
						}

						NextRow = AlternateRow - CenterRow;
						NextCol = AlternateCol - CenterCol;
					}
					else {

						UserMapInformation[a][b] = MissAim;
						//LastFire = Miss; //Lastfire hit gibi gorunmeli cunku sanki merkezi yeni vurmusuz da etrafinda yeni yer ariyormusuz gibi gorunmeli
						//yani merkezimiz var ama yonumuz yok alternate yok, alternate aramaya devam edecegiz

					}
					break;
				}
			}
		}

		//3-)Ilk isabetten sonra yonumuzu de belirlediysek ayni yonde bosluk cikana kadar bir suru atis yapacaksak
		else if (CenterRow != -1 && LastFire == Hit && AlternateRow != -1)
		{
			AlternateRow += NextRow;
			AlternateCol += NextCol;

			if (IsThereANaval(User, AlternateRow, AlternateCol))//Bir sonraki karede gemi varsa atisimizi yapiyoruz
			{

				UserMapInformation[AlternateRow][AlternateCol] = HitAim;
				usernavals[whichnaval].point++;
				LastFire = Hit;
				if (usernavals[whichnaval].point == usernavals[whichnaval].len)
				{
					usernavals[whichnaval].IsDestroyed = true;
					AddToDeadZone2(whichnaval);

					CenterRow = -1; CenterCol = -1;
					AlternateRow = -1; AlternateCol = -1;
					LastFire = Miss;
				}
			}
			else if (UserMapInformation[AlternateRow][AlternateCol] == NotAimed && AlternateRow >= 0 && AlternateRow <= 9 && AlternateCol >= 0 && AlternateCol <= 9 && !DeadZone2[a][b])
			{//Gemi yoksa orasi duvar mi yoksa daha once atis yapilmis bir yer mi diye kontrol etmeliyiz. Haritanin icinde ve bos bir yer ise iskaladi olarak atis yapiyoruz

				LastFire = Miss;
				UserMapInformation[AlternateRow][AlternateCol] = MissAim;

				NextRow *= -1;
				NextCol *= -1;//Yonu ters ceviriyoruz
				AlternateRow = CenterRow;//Alternate degeri center ile ayni yeri gosteriyor
				AlternateCol = CenterCol;//Bundan sonra alternate degerine ters yonumuzu ekleye ekleye gidebiliriz

			}
			else //Haritanin icinde degilse atis falan yapmiyoruz. Hemen yonumuzu degistirip diger yone atis yapmaliyiz.
			{//Once yon bilgilerimizi degistiriyoruz sonra goto fonksiyonu ile 4. else if bloguna gidiyoruz. Boylece atis hakkimizi 
				//elimizde tutmus oluyoruz
				LastFire = Miss;
				NextRow *= -1;
				NextCol *= -1;//Yonu ters ceviriyoruz
				AlternateRow = CenterRow;//Alternate degeri center ile ayni yeri gosteriyor
				AlternateCol = CenterCol;//Bundan sonra alternate degerine ters yonumuzu ekleye ekleye gidebiliriz
				goto YOUAREONWALLSODONTFIREHERE;
			}

		}

		//4-)Ayni yonde ilerlerken karsimiza bosluk ciktiysa bu sefer gemiyi patlatana kadar diger yone dogru atis yapacaksak
		else if (CenterRow != -1 && LastFire == Miss && AlternateRow != -1)
		{
		YOUAREONWALLSODONTFIREHERE:

			AlternateRow += NextRow;
			AlternateCol += NextCol;

			if (IsThereANaval(User, AlternateRow, AlternateCol))
			{

				UserMapInformation[AlternateRow][AlternateCol] = HitAim;
				usernavals[whichnaval].point++;
				LastFire = Hit;
				if (usernavals[whichnaval].point == usernavals[whichnaval].len)
				{
					usernavals[whichnaval].IsDestroyed = true;
					AddToDeadZone2(whichnaval);

					CenterRow = -1; CenterCol = -1;
					AlternateRow = -1; AlternateCol = -1;
					LastFire = Miss;
				}
			}
			else if (!IsThereANaval(User, AlternateRow, AlternateCol) && AlternateRow >= 0 && AlternateRow <= 9 && AlternateCol >= 0 && AlternateCol <= 9 && !DeadZone2[a][b])
			{//Haritanin icindeki bir noktaysa miss olur
				AlternateCol = -1; AlternateRow = -1;
				LastFire = Hit;//Hit yapmamizin sebebi su, atis yaptigimiz eksende gemi yok olmadiysa diger eksene gecmemiz gerek yani yani else if bloklarin 2 numarali olana
				//Oraya girmek icin de LastFire degerini Hit yapmamiz gerekiyor. Yani sanki merkezi ilk defa vurmusuz da simdi gidecek rastgele yon ariyormusuz gibi...
				UserMapInformation[AlternateRow][AlternateCol] = MissAim;
			}
			else
			{
				AlternateCol = -1; AlternateRow = -1;
				LastFire = Hit;
			}
		}


	}
}
bool IsAllNavalsDestroyed(int playername) {
	int cnt;
	cnt = 0;
	if (playername == Computer)
	{
		for (int i = 0; i < 5; i++)
		{
			if (computernavals[i].IsDestroyed)
				cnt++;
		}
		if (cnt == 5)return true;
		else return false;
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			if (usernavals[i].IsDestroyed)
				cnt++;
		}
		if (cnt == 5)return true;
		else return false;
	}
}

void CreateComputerNavals() {
	int col, row;

	for (int i = 0; i < 10; i++)//User gemi yuklerken DeadZone dizisi dolmustu.
		for (int j = 0; j < 10; j++)// Simdi computer gemi dizmeden once ilk olarak tum haritayi bosaltiyoruz
			DeadZone[i][j] = false;

	while (!computernavals[two].IsPlaced && !computernavals[three1].IsPlaced && !computernavals[three2].IsPlaced && !computernavals[four].IsPlaced && !computernavals[five].IsPlaced)
	{
		while (!computernavals[k].IsPlaced)
		{
			col = rand() % 10;
			row = rand() % 10;

			computernavals[k].x = maps[Computer].x + (col*sqrlen);
			computernavals[k].y = maps[Computer].y + (row*sqrlen);

			if (!IsInMap(Computer))SlideIntoMap(Computer);

			if (!IsInDeadZone(Computer))
			{
				computernavals[k].IsPlaced = true;
				AddToDeadZone(Computer);
				k--;
			}
		}
	}
	Drawing();
}

void AddToDeadZone2(int which) {

	int row, col, length;

	if (usernavals[which].direction == horizontal)
	{

		for (length = 0; length < usernavals[which].len; length++)
		{
			col = FindSqrx((usernavals[which].x) + (length*sqrlen));
			row = FindSqry(usernavals[which].y);

			if (row - 1 >= 0 && col - 1 >= 0)	DeadZone2[row - 1][col - 1] = true;
			if (row - 1 >= 0)				DeadZone2[row - 1][col] = true;
			if (row - 1 >= 0 && col + 1 <= 9)	DeadZone2[row - 1][col + 1] = true;
			if (col - 1 >= 0)				DeadZone2[row][col - 1] = true;
			DeadZone2[row][col] = true;
			if (col + 1 <= 9)				DeadZone2[row][col + 1] = true;
			if (row + 1 <= 9 && col - 1 >= 0)	DeadZone2[row + 1][col - 1] = true;
			if (row + 1 <= 9)				DeadZone2[row + 1][col] = true;
			if (row + 1 <= 9 && col + 1 <= 9)	DeadZone2[row + 1][col + 1] = true;

		}
	}
	else
	{
		for (length = 0; length < usernavals[which].len; length++)
		{
			row = FindSqry(usernavals[which].y + (length*sqrlen));
			col = FindSqrx(usernavals[which].x);

			if (row - 1 >= 0 && col - 1 >= 0)	DeadZone2[row - 1][col - 1] = true;
			if (row - 1 >= 0)				DeadZone2[row - 1][col] = true;
			if (row - 1 >= 0 && col + 1 <= 9)	DeadZone2[row - 1][col + 1] = true;
			if (col - 1 >= 0)				DeadZone2[row][col - 1] = true;
			DeadZone2[row][col] = true;
			if (col + 1 <= 9)				DeadZone2[row][col + 1] = true;
			if (row + 1 <= 9 && col - 1 >= 0)	DeadZone2[row + 1][col - 1] = true;
			if (row + 1 <= 9)				DeadZone2[row + 1][col] = true;
			if (row + 1 <= 9 && col + 1 <= 9)	DeadZone2[row + 1][col + 1] = true;

		}
	}


}
void SetProbabilityMap() {

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			ProbabilityMap[i][j] = 0;
		}
	}

	for (int i = 0; i < 5; i++) {
		if (!(usernavals[i].IsDestroyed)) {
			CalculatePMapFor(i);
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			printf("%d ", ProbabilityMap[i][j]);
		}
		printf("\n");
	}

	printf("\n"); printf("\n"); printf("\n");

}

void CalculatePMapFor(int navalnum) {

	int cnt;

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {

			cnt = 0;
			while (cnt != usernavals[navalnum].len) //Geminin uzunlugu ve sigabilecegi bosluk miktari esit olana kadar dongu donecek
			{
				if (UserMapInformation[i][j + cnt] == NotAimed && (j + cnt) < 10) {
					cnt++;
				}
				else break;// Istenilen bosluk miktarina ulasamadan uygun olmayan bir kareye rastlarsak dongu duruyor
			}

			if (cnt == usernavals[navalnum].len) {
				cnt = 0;
				while (cnt != usernavals[navalnum].len)
				{
					ProbabilityMap[i][j + cnt]++;
					cnt++;
				}
			}


		}
	} //Yatayda sigip sigmadigini kontrol ediyor

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {

			cnt = 0;
			while (cnt != usernavals[navalnum].len) //Geminin uzunlugu ve sigabilecegi bosluk miktari esit olana kadar dongu donecek
			{
				if (UserMapInformation[i + cnt][j] == NotAimed && (i + cnt) < 10) {
					cnt++;
				}
				else break;// Istenilen bosluk miktarina ulasamadan uygun olmayan bir kareye rastlarsak dongu duruyor
			}

			if (cnt == usernavals[navalnum].len) {
				cnt = 0;
				while (cnt != usernavals[navalnum].len)
				{
					ProbabilityMap[i + cnt][j]++;
					cnt++;
				}
			}


		}
	}
}