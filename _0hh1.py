from graphics import *
import random

#This game is called 0hh1, there are a few rules
#You must fill the board with tiles
#No two columns or rows can be the same
#There cannot be more than two tiles of the same color adjecent to each other (discounting diagonals)

#This python game was made using graphics.py graphics library created by John Zelle
#If you run this file without having graphics.py downloaded it may not work
#You can find the file here: http://mcsp.wartburg.edu/zelle/python/graphics.py

#sets up a blank 4 by 4 board
board = [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]]

def check_full_board():
    for x in range(4):
        for y in range(4):
            if board[x][y] == 0:
                return False
    return True


#checks to make sure that no two rows are the same (returns true if so)
def check_illegals_same_row():
    if check_full_board() == False:
        return False
    for x in range(4):
        for y in range(4):
            if x != y:
                if board[x][0] == board[y][0] and board[x][1] == board[y][1] and board[x][2] == board[y][2] and board[x][3] == board[y][3]:
                    return False

    return True

#checks to make sure that no two columns are the same (returns true if so)
def check_illegals_same_col():
    if check_full_board() == False:
        return False
    for x in range(4):
        for y in range(4):
            if x != y:
                if board[0][x] == board[0][y] and board[1][x] == board[1][y] and board[2][x] == board[2][y] and board[3][x] == board[3][y]:
                    return False

    return True

#Checks to see if there are three of one color in a row (returns true if not because 3 in one row is an illegal move)
def check_three_in_row():
    red_counter = 0
    blue_counter = 0
    for x in range(4):
        for y in range(4):
            if board[x][y] == 1: 
                red_counter += 1
            if board[x][y] == 2:
                blue_counter += 1
        if red_counter > 2 or blue_counter > 2:
            return False
        red_counter = 0
        blue_counter = 0

    return True

#Checks to see if there are three of one color in a column (returns true if not because 3 in one column is an illegal move)
def check_three_in_col():
    red_counter = 0
    blue_counter = 0
    for y in range(4):
        for x in range(4):
            if board[x][y] == 1: 
                red_counter += 1
            if board[x][y] == 2:
                blue_counter += 1
        if red_counter > 2 or blue_counter > 2:
            return False
        red_counter = 0
        blue_counter = 0
    
    return True

#This function draws the current board upon function call (updates with every mouse click in the window)
def draw_board(win, full_board):
    buffer = 30 #buffer so the board is not against the edge of the window
    
    #start drawing tiles
    for x in range(4):
        for y in range(4):
            temp = Rectangle(Point(buffer + (110*x),buffer+(110*y)+100), Point(buffer + (110*x) + 100, buffer + (110*y)))
            if board[x][y] == 0:
                temp.setFill("white")
                temp.draw(win)
            elif board[x][y] == 1:
                temp.setFill("dark red")
                temp.draw(win)
            elif board[x][y] == 2:
                temp.setFill("blue")
                temp.draw(win)
    #end drawing tiles

    #start drawing check win button (appears with red text while board is not complete, then blue when check function is available)
    check_button = Text(Point(250, 520), 5)
    check_button_out = Rectangle(Point(200,540), Point(300, 500))
    check_button.setText("Check")
    if full_board == False:
        check_button.setTextColor("red")
        check_button_out.setFill("gray")
        check_button_out.draw(win)
        check_button.draw(win)
    elif full_board == True:
        check_button.setTextColor("blue")
        check_button.setStyle("bold")
        check_button_out.setFill("light gray")
        check_button_out.draw(win)
        check_button.draw(win)
    #end drawing check win button
    return

#Uses the above functions to check if the player has won the game
def check_win():
    #print(check_illegals_same_row(), " ", check_illegals_same_col(), " ", check_three_in_row(), " ", check_three_in_col())
    if check_three_in_col() and check_three_in_row() and check_illegals_same_row() and check_illegals_same_col():
        return True

    return False

def draw_win_screen(win):
    end = Rectangle(Point(0,0), Point(500,550))
    end.setFill("white")
    end.draw(win)
    You = Text(Point(230, 225), 10)
    Win = Text(Point(240, 275), 10)
    You.setTextColor("red")
    Win.setTextColor("blue")
    You.setText("YOU")
    Win.setText("WIN!")
    You.draw(win)
    Win.draw(win)
    return

#core gameplay mechanics
def main():
    #start initializing board
    init_x = 0
    init_y = 0
    blue_red = 0
    full_board = False
    counter = 0
    while counter < 5:
        init_x = random.randint(0,3)
        init_y = random.randint(0,3)
        blue_red = random.randint(1,2)
        board[init_x][init_y] = blue_red
        if check_three_in_col() == False or check_three_in_row() == False:
            board[init_x][init_y] = 0
            counter -= 1
        counter += 1
    #end initialization

    #initialize window
    win = GraphWin("0hh1", 500, 550)
    win.setBackground("white")
    playing = True
    while playing == True:
        full_board = check_full_board()
        #draw and update the board
        draw_board(win, full_board)
        #get position of mouse upon click
        mouse = win.getMouse()
        col = -1
        row = -1
        if mouse.x >= 30 and mouse.x <= 130:
            col = 0
        elif mouse.x >= 140 and mouse.x <= 240:
            col = 1
        elif mouse.x >= 250 and mouse.x <= 350:
            col = 2
        elif mouse.x >= 360 and mouse.x <= 460:
            col = 3
        if mouse.y >= 30 and mouse.y <= 130:
            row = 0
        elif mouse.y >= 140 and mouse.y <= 240:
            row = 1
        elif mouse.y >= 250 and mouse.y <= 350:
            row = 2
        elif mouse.y >= 360 and mouse.y <= 460:
            row = 3
        if col != -1 and row != -1:
            board[col][row] += 1
            if board[col][row] == 3:
                board[col][row] = 0
        if (0 not in board):
            full_board = True
        if mouse.x >= 200 and mouse.x <= 300 and mouse.y >= 500 and mouse.y <= 540 and full_board == True:
            if check_win() == True:
                draw_win_screen(win)
                playing = False
                win.getMouse()
                win.close()
    return

#run main
main()