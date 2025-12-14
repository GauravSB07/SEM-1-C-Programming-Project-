//-------LIBRARIES--------//

#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dos.h>
#include <string.h>

//---------CONFIGURATION----------//
#define CELL   40
#define GRID_X 40
#define GRID_Y 60
#define THICK_W 3

//----------UNDO STACK-------//
#define UNDO_MAX 1024

//----------GLOBALS----------//

int undo_r[UNDO_MAX], undo_c[UNDO_MAX], undo_val[UNDO_MAX];
int undo_top = 0;

int board[9][9];
int solution[9][9];
int fixed_cell[9][9];
int hinted_cell[9][9]={0};


int sel_r = 0, sel_c = 0;
time_t start_time;
int last_sec = -1;
int holes_current = 35;
int level_no = 1;
int mistakes = 0;
int max_mistakes = 20;
int hints_used = 0;
int max_hints = 3;
int threshold = 10*60;

//----------PROTOTYPES---------//
int for_row(int grid[9][9],int row,int num);
int for_col(int grid[9][9],int col,int num);
int for_box(int grid[9][9],int br,int bc,int num);
int is_safe(int grid[9][9], int r, int c, int num);
int fill_grid(int grid[9][9]);
int solve_grid(int grid[9][9]);

void copy_grid(int src[9][9], int dst[9][9]);
void generate_new_puzzle(int holes);

void draw_all(void);
void draw_grid(void);
void draw_numbers(void);
void draw_static_hud(void);
void draw_hud(void);
void draw_selected(void);
void give_hint(void);
void flash_cell(int r, int c);

void push_undo(int r, int c, int oldv);
int pop_undo(void);
int check_violation(int r,int c);
int check_complete(void);

void show_center(const char *s, int ms);
void show_solution(void);
void redraw_cell(int r, int c);



//-----------SUDOKU LOGIC-----------//

int for_row(int grid[9][9], int row, int num)
{
    int c;
    for (c = 0; c < 9; c++)
    {
    if (grid[row][c] == num)
	return 1;
    }
    return 0;
}

int for_col(int grid[9][9], int col, int num)
{
    int r;
    for (r = 0; r < 9; r++)
       {
       if(grid[r][col] == num)
	return 1 ;
       }
    return 0;
}

int for_box(int grid[9][9], int startRow, int startCol, int num)
{
    int r, c;
    for (r = 0; r < 3; r++)
	for (c = 0; c < 3; c++)
	    {
	    if (grid[startRow + r][startCol + c] == num)
		return 1;
	    }
    return 0;
}

int is_safe(int grid[9][9], int r, int c, int num)
{
    return !for_row(grid,r,num) &&
	   !for_col(grid,c,num) &&
	   !for_box(grid, r - r%3, c - c%3, num);
}

void copy_grid(int src[9][9],int dst[9][9])
{
    int r,c;
    for (r=0; r<9; r++)
	 {
	  for (c=0; c<9; c++)
	      {
	      dst[r][c]=src[r][c];
	       }
	 }
}

int fill_grid(int grid[9][9])
{
    int r,i,c, nums[9];

    for (r = 0; r < 9; r++) 
    {
	for (c = 0; c < 9; c++) 
        {
	    if (grid[r][c] == 0) goto found_empty;
	}
    }
    return 1;

   found_empty:

    for (i = 0; i < 9; i++) 
         {
          nums[i] = i + 1;
         }

    for ( i = 8; i > 0; i--) 
    {
	int j = rand() % (i + 1);
	int temp = nums[i];
	nums[i] = nums[j];
	nums[j] = temp;
    }

    for ( i = 0; i < 9; i++)
    {
	if (is_safe(grid, r, c, nums[i]))
	{
	    grid[r][c] = nums[i];
	    if (fill_grid(grid))
	    return 1;
	    grid[r][c] = 0;
	}
    }
    return 0;

}

int solve_grid(int grid[9][9]) 
{
    int row, col;
    int r, c;
    int empty_cell_found = 0;
    int number;

    row = -1;
    col = -1;

    for (r=0; r < 9 && !empty_cell_found; r++)
    {
	for(c = 0; c < 9; c++)
	{
	    if (grid[r][c] == 0)
	    {
		row = r;
		col = c;
		empty_cell_found = 1;
		break;
	    }
	}

    }

    if (!empty_cell_found) 
    {
	return 1;
    }

    for (number = 1; number <= 9; number++) 
    {
	if (is_safe(grid, row, col, number))
            {
	    grid[row][col] = number;

	    if (solve_grid(grid)) 
            {
		return 1;
	    }

	    grid[row][col] = 0;
	}
    }

    return 0;
}

void generate_new_puzzle(int holes) 
{
    int r,c,row;
    int backup;
    int removed=0;
    int temp[9][9];

    for(r = 0; r < 9; r++)
    {
	for (c = 0; c < 9; c++)
	{
	    solution[r][c] = 0;
	}
    }

    fill_grid(solution);

    copy_grid(solution, board);

    for(r = 0; r < 9; r++)
    {
	for (c = 0; c < 9; c++)
	{
	    fixed_cell[r][c] = 1;
	}
    }

    while (removed < holes)
    {
	r = rand() % 9;
	c = rand() % 9;

	if (board[r][c] != 0)
	  {
	    backup = board[r][c];
	    board[r][c] = 0;

	     copy_grid(board, temp);

	    if (solve_grid(temp))
	    {
		fixed_cell[r][c] = 0;
		removed++;
	    }
	    else
	    {
		board[r][c] = backup;
	    }
	  }
    }

    for (r=0;r<9;r++)
     {
	for (c=0;c<9;c++)
	 {
	    if (board[r][c]!=0)
	    {
		 fixed_cell[r][c]=1;
	    }
	  }
     }

if (level_no == 1) max_hints = 3;
else if (level_no == 2) max_hints = 3;
else if (level_no == 3) max_hints = 2;
else if (level_no == 4) max_hints = 2;
else if (level_no == 5) max_hints = 1;
else if (level_no == 6) max_hints = 1;
else max_hints = 1;

hints_used = 0;

    undo_top = 0;
    sel_r = sel_c = 0;
    start_time = time(NULL);
    mistakes = 0;

threshold = (10*60)-((level_no -1)*45);
if(level_no>7)threshold = 5*60+30;
}

//----------DRAWING-----------//
void draw_static_hud(void)
{
    int right_x=GRID_X+9*CELL+40;
    int top_y=GRID_Y;

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    setcolor(BLACK);
    outtextxy(right_x,  top_y+ 140, "=== CONTROLS ===");

    outtextxy(right_x,  top_y + 160, "Arrows = Move");
    outtextxy(right_x,  top_y + 180, "1-9 = Place Number");
    outtextxy(right_x,  top_y + 200, "Backspace = Clear");
    outtextxy(right_x,  top_y + 220, "U = Undo");
    outtextxy(right_x,  top_y + 240, "S = Submit");
    outtextxy(right_x, top_y + 260, "H = Show Solution");
    outtextxy(right_x, top_y + 280, "N= Give Hint");
    outtextxy(right_x,  top_y + 300, "E / Esc = Exit");
}

void draw_hud(void)
{
    time_t nowt = time(NULL);
    int elapsed;
    char buf[64];
    int right_x=GRID_X + 9*CELL + 40;

    int top_y=GRID_Y;

    settextstyle(DEFAULT_FONT,HORIZ_DIR,1);
    setcolor(BLACK);

    elapsed = (int)difftime(nowt, start_time);

    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(right_x-5,top_y+15,right_x+200,top_y+90);

    sprintf(buf, "Level: %d", level_no);
    outtextxy(right_x, top_y+ 10, buf);

    sprintf(buf, "Holes: %d", holes_current);
    outtextxy(right_x, top_y + 30, buf);

    sprintf(buf, "Time: %02d:%02d", elapsed / 60, elapsed % 60);
    outtextxy(right_x, top_y + 50, buf);

    sprintf(buf, "Mistakes: %d/%d", mistakes, max_mistakes);
    outtextxy(right_x, top_y + 70, buf);

    sprintf(buf,"Time Limit: %02d:%02d", threshold/60, threshold%60);
    outtextxy(right_x,top_y+90,buf);

    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(right_x - 10, top_y + 105, right_x + 110, top_y + 125);
    setcolor(BLACK);

    sprintf(buf,"Hints: %d/%d",hints_used,max_hints);
    outtextxy(right_x,top_y+110,buf);

}

// ================== DRAW GRID ==================//
void draw_grid(void)
{
    int i;
    int right = GRID_X + 9*CELL;
    int bottom = GRID_Y + 9*CELL;
    int half = THICK_W/2;

    setfillstyle(SOLID_FILL,LIGHTGRAY);
    bar(0, 0, getmaxx(), getmaxy());

    setfillstyle(SOLID_FILL, WHITE);
    bar(GRID_X, GRID_Y, right, bottom);

    setcolor(BLACK);
    setlinestyle(SOLID_LINE, 0, 1);
    for (i = 1; i < 9; i++)
    {
	if (i % 3 != 0)
	{
	    line(GRID_X + i * CELL, GRID_Y, GRID_X + i * CELL, bottom);
	    line(GRID_X, GRID_Y + i * CELL, right, GRID_Y + i * CELL);
	}
    }

setfillstyle(SOLID_FILL, BLACK);
    for (i = 0; i <= 9; i += 3)
    {

	bar(GRID_X + i*CELL - half, GRID_Y,
	    GRID_X + i*CELL + half, bottom);

	bar(GRID_X, GRID_Y + i*CELL - half,
	    right, GRID_Y + i*CELL + half);
    }

    setlinestyle(SOLID_LINE,0,1);
}

void redraw_cell(int r, int c)
{
    int x = GRID_X + c*CELL;
    int y = GRID_Y + r*CELL;
    char s[3];

    setfillstyle(SOLID_FILL, WHITE);
    bar(x+3,y+3,x+CELL-3,y+CELL-3);

    if (board[r][c]!=0)
    {
	sprintf(s,"%d", board[r][c]);
	if (fixed_cell[r][c]) setcolor(BLUE);
	else if(hinted_cell[r][c]) setcolor(GREEN);
	else if (check_violation(r,c)) setcolor(RED);
	else setcolor(BLACK);

	settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
	outtextxy(x + CELL/2 - textwidth(s)/2,
		  y + CELL/2 - textheight(s)/2, s);
    }
}

void draw_numbers(void)
{
    int r,c;
    for (r=0;r<9;r++) for (c=0;c<9;c++) redraw_cell(r,c);
}

void draw_selected(void)
{
    int x = GRID_X + sel_c*CELL;
    int y = GRID_Y + sel_r*CELL;

    setcolor(RED);
    setlinestyle(SOLID_LINE, 0, 1);

    rectangle(x + 3, y + 3, x + CELL - 3, y + CELL - 3);
}

void draw_all(void)
{
    draw_grid();
    draw_numbers();
    draw_selected();
}


void show_center(const char *s, int ms)
{
    int mx=getmaxx(), my=getmaxy();
    int bx = mx/2 - 170, by = my/2 - 30;
    int bw = 340, bh = 60;
    void *buf;
    int size;

    size = imagesize(bx, by, bx + bw, by + bh);
    buf = malloc(size);
    if (!buf) return;

    getimage(bx, by, bx + bw, by + bh, buf);

    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(bx,by,bx+bw,by+bh);
    setcolor(BLACK); rectangle(bx,by,bx+bw,by+bh);
    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
    outtextxy(bx+10,by+18,(char*)s);

    delay(ms);

    putimage(bx, by, buf, COPY_PUT);
    free(buf);
}

void show_solution(void)
{
    int r,c;
    draw_grid();
    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
    setcolor(BLACK);

    for (r=0;r<9;r++) for (c=0;c<9;c++)
    {
	char s[3];
	sprintf(s,"%d",solution[r][c]);
	outtextxy(GRID_X + c*CELL + CELL/2 - textwidth(s)/2,
		  GRID_Y + r*CELL + CELL/2 - textheight(s)/2, s);
    }

    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
    setcolor(RED);
    outtextxy(410,200,"You failed");
    outtextxy(410,230,"this level!");
    outtextxy(420,260,"Restarting");
    delay(2000);

    mistakes = 0;
    start_time = time(NULL);
    last_sec = -1;

    generate_new_puzzle(holes_current);
    draw_all();
    draw_static_hud();
    draw_hud();
}

//----------UNDO & CHECKS-----------//

void push_undo(int r, int c, int oldv)
 {
    if (undo_top < UNDO_MAX)
    {
	undo_r[undo_top] = r;
	undo_c[undo_top] = c;
	undo_val[undo_top] = oldv;
	undo_top++;
    }
}

int pop_undo(void)
{
    int r,c;
    if (undo_top <= 0) return 0;
    undo_top--;
    r = undo_r[undo_top];
    c = undo_c[undo_top];
    board[r][c] = undo_val[undo_top];
    return 1;
}

int check_violation(int r, int c)
{
    int num = board[r][c];
    if (num == 0) return 0;
    board[r][c] = 0;
    {
	int conflict = for_row(board,r,num) || for_col(board,c,num) ||
		       for_box(board, r - r%3, c - c%3, num);
	board[r][c] = num;
	return conflict;
    }
}

int check_complete(void)
{
    int r,c;
    for (r=0;r<9;r++) for (c=0;c<9;c++)
    {
	if (board[r][c]==0) return 0;
	if (board[r][c]!=solution[r][c]) return 0;
    }
    return 1;
}

int is_incomplete()
{
    int r, c;
    for (r = 0; r < 9; r++)
    {
	for (c = 0; c < 9; c++)
	{
	    if (board[r][c] == 0)
	    {
		return 1;
	    }
	}
    }
    return 0;
}

void flash_cell(int r, int c)
{
    int i, x = GRID_X + c*CELL, y = GRID_Y + r*CELL;
    for (i=0;i<3;i++)
    {
	setfillstyle(SOLID_FILL, RED);
	bar(x+1,y+1,x+CELL-1,y+CELL-1);
	setcolor(BLACK); rectangle(x,y,x+CELL,y+CELL);
	delay(120);

	setfillstyle(SOLID_FILL, WHITE);
	bar(x+1,y+1,x+CELL-1,y+CELL-1);
	setcolor(BLACK); rectangle(x,y,x+CELL,y+CELL);
	delay(120);
    }

    if (board[r][c])
    {
	char s[3]; sprintf(s,"%d",board[r][c]);
	settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
	outtextxy(x + CELL/2 - textwidth(s)/2,
		  y + CELL/2 - textheight(s)/2, s);
    }
}
void give_hint()
{
    char msg[50];

    if (hints_used >= max_hints)
    {
	show_center("No hints left!", 1000);
	return;
    }

    if (fixed_cell[sel_r][sel_c])
    {
	show_center("This cell is fixed!", 1000);
	return;
    }

    if (board[sel_r][sel_c] == solution[sel_r][sel_c] && hinted_cell[sel_r][sel_c] == 1)
    {
	show_center("Hint already used!", 1000);
	return;
    } else if (board[sel_r][sel_c] == solution[sel_r][sel_c])
    {
	show_center("Cell already correct!", 1000);
	return;
    }

    board[sel_r][sel_c] = solution[sel_r][sel_c];

    hinted_cell[sel_r][sel_c] = 1;

    flash_cell(sel_r, sel_c);

    redraw_cell(sel_r, sel_c);

    draw_selected();

    hints_used++;

    sprintf(msg, "Hint used: %d/%d", hints_used, max_hints);
    show_center(msg, 900);
}
//-------------------MAIN--------------//
int main(void)
{
    int gd = DETECT, gm = 0;
    int ch, num, r, c;
    time_t nowt;
    int elapsed, last_sec = -1;
    int threshold;
    char msg[80];

    srand((unsigned)time(NULL));
    initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");

    generate_new_puzzle(holes_current);
    draw_all();
    draw_static_hud();
    draw_hud();

    while (1)
    {
	nowt = time(NULL);
	elapsed = (int)difftime(nowt, start_time);

	if (elapsed != last_sec)
	{
	    draw_hud();
	    last_sec = elapsed;
	}

	if (mistakes >= max_mistakes)
	{
	    show_center("Too many mistakes! Restarting level...", 1500);
	    mistakes = 0;
	    generate_new_puzzle(holes_current);
	    draw_all();
	    draw_static_hud();
	    draw_hud();
	    last_sec = -1;
	    continue;
	}

	if (kbhit())
	{
	    ch = getch();

	    if (ch == 0 || ch == 224)
	    {
		ch = getch();
		redraw_cell(sel_r, sel_c);

		if (ch == 72 && sel_r > 0)
		    sel_r--;
		else if (ch == 80 && sel_r < 8)
		    sel_r++;
		else if (ch == 75 && sel_c > 0)
		    sel_c--;
		else if (ch == 77 && sel_c < 8)
		    sel_c++;
		draw_selected();
	    }

	    else if (ch >= '1' && ch <= '9')
	    {
		num = ch - '0';
		if (!fixed_cell[sel_r][sel_c])
		{
		    if (is_safe(board, sel_r, sel_c, num))
		    {
			push_undo(sel_r, sel_c, board[sel_r][sel_c]);
			board[sel_r][sel_c] = num;
		    }
		    else
		    {
			mistakes++;
			show_center("Invalid move", 700);
		    }
		}
		else
		{
		    show_center("Cell is fixed", 800);
		}

		redraw_cell(sel_r, sel_c);
		draw_selected();
	    }

	    else if (ch == 8)
	    {
		if (!fixed_cell[sel_r][sel_c])
		{
		    push_undo(sel_r, sel_c, board[sel_r][sel_c]);
		    board[sel_r][sel_c] = 0;
		}
		else
		{
		    show_center("Cell is fixed", 800);
		}

		redraw_cell(sel_r, sel_c);
		draw_selected();
	    }

	    else if (ch == 'u' || ch == 'U')
	    {
		if (!pop_undo())
		    show_center("Nothing to undo", 900);

		draw_numbers();
		draw_selected();
	    }
	    else if (ch == 'h' || ch == 'H')
	    {
		show_solution();
	    }
	    else if (ch == 'n' || ch == 'N')
	    {
		give_hint();
		draw_hud();
		draw_all();
		draw_static_hud();
	    }
	    else if (ch == 's' || ch == 'S')
	    {
		if (check_complete())
		{
		    nowt = time(NULL);
		    elapsed = (int)difftime(nowt, start_time);

		    sprintf(msg, "You Won! Time %02d:%02d",
			    elapsed/60, elapsed%60);
		    show_center(msg, 1400);

		    threshold = 7 * 60;
		    if (elapsed <= threshold && mistakes < max_mistakes)
		    {
			show_center("Next level unlocked!", 1200);
			level_no++;
			holes_current += 5;
			if (holes_current > 64)
			    holes_current = 64;

			max_mistakes -= 2;
				    if (max_mistakes < 5) max_mistakes = 5;

			generate_new_puzzle(holes_current);
			draw_all();
			draw_static_hud();
			draw_hud();
			last_sec = -1;
		    }
		    else
		    {
			show_center("Solved, but too slow ", 1400);
		    }
		}
		else
		{
		    int anyv = 0;
		    for (r = 0; r < 9; r++)
		    {
			for (c = 0; c < 9; c++)
			{
			    if (board[r][c] != 0 && check_violation(r, c))
			    {
				flash_cell(r, c);
				anyv = 1;
			    }
			}
		    }

		    if (!anyv)
			show_center("Incomplete puzzle", 1100);

		    draw_numbers();
		    draw_selected();
		}
	    }
	    else if (ch == 'e' || ch == 'E' || ch == 27)
	    {
		closegraph();
		return 0;
	    }
            else {
		    int anyv = 0;
		    for (r=0; r<9; r++) for (c=0; c<9; c++) 
                        {
			if (board[r][c]!=0 && check_violation(r,c)) 
                        {
			    flash_cell(r,c);
			    anyv = 1;
			}
		  }
		    if (!anyv) show_center("Enter a valid key", 1100);
		    draw_numbers();
		    draw_selected();
		}
	    
        
        }

        delay(25);
    }
}
