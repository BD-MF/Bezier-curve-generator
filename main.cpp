//  Created by Manohar Boppana on 12/5/18.
//  Copyright © 2018 Manohar Boppana. All rights reserved.

#define GL_SILENCE_DEPRECATION //Defined to avoid deprication messages
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h> 
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Point {
public:
    float x, y;
    Point(){};
    Point(float x, float y):x(x),y(y){};
};

class BCurve{
public:
    int pNum;
    vector<Point> pts;
};

class Spline{
public:
    int pNum, kNum, k;
    vector<Point> pts;
    vector<int> knots;
    
};

int PointNum, curIn = 1, res = 100, resSpline = 100, bcNum, splineNum;
float Width = 750, Height = 750;

vector<BCurve> bcObjs;
vector<Spline> splineObjs;

//reads input from a file
int Read(string filename)
{
    ifstream input(filename);
    Spline s;
    BCurve b;
    if(input.is_open())
    {
        input >> bcNum;
        
        for(int i = 0; i < bcNum; i++)
        {
            input >> b.pNum;
            Point p;
            for(int j = 0; j < b.pNum; j++)
            {
                input >> p.x >> p.y;
                b.pts.push_back(p);
            }
            
            bcObjs.push_back(b);
            b.pts.clear();
        }
        cout << "here" << endl;
        input >> splineNum;
        
        for(int i = 0; i < splineNum; i++)
        {
            input >> s.pNum;
            Point p;
            int knot;
            
            for(int j = 0; j < s.pNum; j++)
            {
                input >> p.x >> p.y;
                s.pts.push_back(p);
            }
            
            input >> s.k;
            s.kNum = s.pNum+s.k;
            
            for(int j = 0; j < s.kNum; j++)
            {
                input >> knot;
                s.knots.push_back(knot);
            }
            
            splineObjs.push_back(s);
            s.pts.clear();
            s.knots.clear();
        }
    }
    else
    {
        cout << "File Not Open!!" << endl;
        return 0;
    }
    return 1;
}

//writes back to the destination file
int Write(string filename)
{
    ofstream output(filename);
    if(output.is_open())
    {
        output << bcNum << "\n" << endl;
        
        for(int i = 0; i < bcObjs.size(); i++)
        {
            output << bcObjs[i].pts.size() << endl;
            for(int j = 0; j < bcObjs[i].pts.size(); j++)
            {
                output << bcObjs[i].pts[j].x << " " << bcObjs[i].pts[j].y << endl;
            }
        }
        
        output << "\n" << splineNum << "\n" << endl;
        
        for(int i = 0; i < splineObjs.size(); i++)
        {
            output << splineObjs[i].pts.size() << endl;
            
            for(int j = 0; j < splineObjs[i].pts.size(); j++)
            {
                output << splineObjs[i].pts[j].x << " " << splineObjs[i].pts[j].y << endl;
            }
            
            output << splineObjs[i].k << endl;
            
            for(int j = 0; j < splineObjs[i].knots.size(); j++)
            {
                output << splineObjs[i].knots[j] << endl;
            }
        }
    }
    else{
        cout << "File Not Open!!" << endl;
        return 0;
    }
    return 1;
}

//convert to NDC
float NDC(float x)
{
    float WinDelta = max (Width, Height);
    x = (((x - 0.0)/WinDelta)*2) - 1;
    return x;
}

//draw control points
void drawPoints(vector<Point> points)
{
    glPointSize(10);
    glBegin(GL_POINTS);
    for(int i = 0; i < points.size(); i++){
        float x,y,xNDC,yNDC;
        x = points[i].x;
        y = points[i].y;
        
        xNDC = NDC(x);
        yNDC = NDC(y);
        
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(xNDC,yNDC);
    }
    glEnd();
}

//join the control points with line segments
void drawLines(vector<Point> points)
{
    glBegin(GL_LINES);
    for(int i = 0; i < points.size()-1 ; i++){
        float x1,y1,x2,y2,x1NDC,y1NDC,x2NDC,y2NDC;
        x1 = points[i].x;
        y1 = points[i].y;
        x2 = points[i+1].x;
        y2 = points[i+1].y;
        
        x1NDC = NDC(x1);
        x2NDC = NDC(x2);
        y1NDC = NDC(y1);
        y2NDC = NDC(y2);
        
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(x1NDC,y1NDC);
        glVertex2f(x2NDC,y2NDC);
    }
    glEnd();
}

//function to delete control points
void deletePoint(vector<Point> &points)
{
    int x;
    
    cout << "\nEnter the index of the vertex to be deleted: ";
    cin >> x;
    
    points.erase(points.begin() + x-1);
}

//function to insert control points
void insertPoint(vector<Point> &points)
{
    int x;
    Point p;
    
    cout << "\nEnter the index for the vertex to be inserted: ";
    cin >> x;
    cout << "Enter the new x-coordinate: ";
    cin >> p.x;
    cout << "Enter the new y-coordinate: ";
    cin >> p.y;
    
    points.insert(points.begin() + x-1, p);
    
}

//function to add control points
void addPoint(vector<Point> &points)
{
    Point p;
    
    cout << "\nEnter the new x-coordinate: ";
    cin >> p.x;
    cout << "Enter the new y-coordinate: ";
    cin >> p.y;
    
    points.push_back(p);
    
}

//function to modify control points
void modifyPoint(vector<Point> &points)
{
    int x;
    
    cout << "\nEnter the index for the vertex to be modified: ";
    cin >> x;
    cout << "Enter the new x-coordinate: ";
    cin >> points[x-1].x;
    cout << "Enter the new y-coordinate: ";
    cin >> points[x-1].y;
}

//deCasteljau algorithm for Bezier curves
void deCasteljau(float t, vector<Point> &v, vector<Point> points)
{
    vector<vector<Point> > vecJ;
    vector<Point> vecI;
    Point p;
    
    vecI = points;
    vecJ.push_back(vecI);
    int n = points.size();
    vecI.clear();
    
    for(int j = 1; j < n; j++)
    {
        for(int i = 0; i < n-j; i++)
        {
            p.x = ((1 - t) * vecJ[j-1][i].x) + (t * vecJ[j-1][i+1].x);
            p.y = ((1 - t) * vecJ[j-1][i].y) + (t * vecJ[j-1][i+1].y);
            
            vecI.push_back(p);
        }
        vecJ.push_back(vecI);
        vecI.clear();
    }
    
    v.push_back(vecJ[n-1][0]);
}

//draw the Bezier curve
void drawCurve(vector<Point> v)
{
    glBegin(GL_LINES);
    for(int i = 0; i < v.size() - 1; i++){
        float x1,y1,x2,y2, x1NDC,y1NDC,x2NDC,y2NDC;;
        x1 = v[i].x;
        y1 = v[i].y;
        x2 = v[i+1].x;
        y2 = v[i+1].y;
        
        x1NDC = NDC(x1);
        x2NDC = NDC(x2);
        y1NDC = NDC(y1);
        y2NDC = NDC(y2);
        
        glColor3f(0.f, 0.f, 1.f);
        glVertex2f(x1NDC,y1NDC);
        glVertex2f(x2NDC,y2NDC);
    }
    glEnd();
}

//function to get the curve points for Bezier curve
void Bezier(vector<Point> points)
{
    vector<Point> bcpoints;
    float t;
    
    for(int r = 0; r < res; r++)
    {
        t = static_cast<float>(r)/static_cast<float>(res-1);
        
        deCasteljau(t, bcpoints, points);
    }
    
    drawCurve(bcpoints);
    
    bcpoints.clear();
    glutPostRedisplay();
    
}

//function to get input to edit the control points
void BezierFunc()
{
    int bfInput, ID;
    char c;
    cout << "Enter the ID of the Bezier curve: ";
    cin >> ID;
    cout << "\nWould you like to modify the control points? (Y/N)";
    cin >> c;
    
    if(c == 'Y' || c == 'y')
    {
        cout << "\nBezier curve functions - 1. ADD 2. INSERT 3. MODIFY 4. DELETE: ";
        cin >> bfInput;
        
        switch (bfInput) {
            case 1:
                addPoint(bcObjs[ID].pts);
                break;
            case 2:
                insertPoint(bcObjs[ID].pts);
                break;
            case 3:
                modifyPoint(bcObjs[ID].pts);
                break;
            case 4:
                deletePoint(bcObjs[ID].pts);
            default:
                break;
        }
    }
    else if (c == 'N' || c == 'n')
    {
        
    }
    
    cout << "Enter the resolution: ";
    cin >> res;
}

//function to get input to edit the control points
void BsplineFunc()
{
    int bfInput, ID;
    char c;
    
    cout << "Enter the ID of the Bspline curve: ";
    cin >> ID;
    
    cout << "\nWould you like to modify the control points? (Y/N)";
    cin >> c;
    
    if(c == 'Y' || c == 'y')
    {
        cout << "\nBspline curve functions - 1. ADD 2. INSERT 3. MODIFY 4. DELETE: ";
        cin >> bfInput;
        
        switch (bfInput) {
            case 1:
                addPoint(splineObjs[ID].pts);
                break;
            case 2:
                insertPoint(splineObjs[ID].pts);
                break;
            case 3:
                modifyPoint(splineObjs[ID].pts);
                break;
            case 4:
                deletePoint(splineObjs[ID].pts);
            default:
                break;
        }
    }
    else if (c == 'N' || c == 'n')
    {
        
    }
    
    cout << "Enter the resolution: ";
    cin >> resSpline;
}

//de Boor algorithm
void deBoor(float U, vector<Point> &v, Spline s)
{
    int domLow = 0, domHigh = 0, divs= 0, ulowInt= 0, uHighInt= 0;
    float uInterval= 0.0;
    vector<vector<Point> > vecJ;
    vector<Point> vecI;
    vector<float> u;
    Point p;
    
    vecI = s.pts;
    vecJ.push_back(vecI);
    vecI.clear();
    
    domLow = s.k -1;
    domHigh = s.pNum;
    divs = domHigh - domLow;
    uInterval = 1.0 / static_cast<float>(divs);
    
    for(int i = domLow; i < domHigh; i++)
    {
        if (U >= i && U < i+1)
        {
            ulowInt = i;
            uHighInt = i+1;
        }
        
    }
    
    for(int i = 0; i < s.pNum+s.k; i++)
    {
        u.push_back(i);
    }
    
    for(int j = 1; j < domLow+1; j++)
    {
        for (int i = 0; i < ulowInt - domLow; i++)
        {
            vecI.push_back(Point(0.0,0.0));
        }
        
        for(int i = (ulowInt - domLow); i < ulowInt - j + 1; i++)
        {
            float a1, a2;
            
            a1 = (u[i+s.k] - U) / (u[i+s.k] - u[i+j]);
            a2 = (U - u[i+j]) / (u[i+s.k] - u[i+j]);
            
            p.x = (a1 * vecJ[j-1][i].x) + (a2 * vecJ[j-1][i+1].x);
            p.y = (a1 * vecJ[j-1][i].y) + (a2 * vecJ[j-1][i+1].y);
            
            vecI.push_back(p);
        }
        vecJ.push_back(vecI);
        vecI.clear();
    }
    
    v.push_back(vecJ[s.k-1][ulowInt - domLow]);
    
}

void Bspline(Spline s)
{
    vector<Point> splvec;
    float r, inc;
    r = s.pts.size() - (s.k-1);
    inc = r/resSpline;
    
    for(int i = 0; i < resSpline; i++)
    {
        deBoor(r, splvec, s);
        r += inc;
    }
    
    drawCurve(splvec);
    splvec.clear();
}

void display()
{
    //Misc.
    glClear(GL_COLOR_BUFFER_BIT);
    
    for(int i = 0; i < bcObjs.size(); i++){
        drawPoints(bcObjs[i].pts);
        drawLines(bcObjs[i].pts);
        Bezier(bcObjs[i].pts);
    }
    
    for(int i = 0; i < splineObjs.size(); i++){
        drawPoints(splineObjs[i].pts);
        drawLines(splineObjs[i].pts);
        Bspline(splineObjs[i]);
    }
    
    //window refresh
    glFlush();
}

//function to handle input
void idle()
{
    cout << "Curves 1.Bezier curve 2.B-Spline curves (Press any other key to EXIT): " ;
    cin >> curIn;
    
    if(curIn == 1)
    {
        BezierFunc();
    }
    else if (curIn == 2)
    {
        BsplineFunc();
    }
    else
        exit(0);
    
    Write("write_file.txt");
    
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    Read("read_file.txt");
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA );
    //set window size to 500*500
    glutInitWindowSize(Width, Height);
    //set window position
    glutInitWindowPosition(500, 100);
    glutCreateWindow("Project 4");
    
    glClearColor (1,1,1,0);
    
    //sets display function
    glutDisplayFunc(display);
    //handles input
    glutIdleFunc(idle);
    
    Write("write_file.txt");
    glutMainLoop();//main display loop, will display until terminate
    return 1;
}

