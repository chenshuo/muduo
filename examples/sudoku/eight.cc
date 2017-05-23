#include "eight.h"

#include <assert.h>
#include <string.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdio.h>

using namespace muduo;

struct Node
{
    int maze[3][3];
    int fun_h,fun_g;
    int pos_x,pos_y;
    int Hash;
    bool operator<(const Node nt)const{
        return fun_h+fun_g>nt.fun_h+nt.fun_g;
    }

    bool check()
    {
        if(pos_x>=0 && pos_x<3 && pos_y>=0 && pos_y<3)
            return true;
        return false;
    }
};

const int MAXNUM=370000;
int HASH[9]={1,1,2,6,24,120,720,5040,40320};//0!~8!
int dest=46233;
int vis[MAXNUM];
int pre[MAXNUM];
int way[4][2]={{0,1},{0,-1},{1,0},{-1,0}};//4 ways

class EightSolver
{
    public:
        EightSolver(string s)
        {
            memset(vis,-1,sizeof(vis));
            memset(pre,-1,sizeof(pre));
            int k=0;
            for(int i=0;i<3;i++)
                for(int j=0;j<3;j++)
                {
                    if((s[k]<='9'&&s[k]>='0')||s[k]=='x')
                    {
                        if(s[k]=='x')
                        {
                            begNode.maze[i][j]=0;
                            begNode.pos_x=i;
                            begNode.pos_y=j;
                        }
                        else
                            begNode.maze[i][j]=s[k]-'0';
                    }

                    k++;
                }
            begNode.Hash=getHash(begNode);
            begNode.fun_g=0;
            begNode.fun_h=getH(begNode);
            vis[begNode.Hash]=1;

        }
        string getAns()
        {
            string ans="";
            int nxt=dest;
            while(pre[nxt]!=-1)
            {
                switch(vis[nxt])
                {
                    case 0:ans+='r';break;
                    case 1:ans+='l';break;
                    case 2:ans+='d';break;
                    case 3:ans+='u';break;
                }
                nxt=pre[nxt];
            }
            reverse(ans.begin(),ans.end());
            return ans;
        }
        bool isOK()
        {
            std::vector<int> v;
            for(int i=0;i<3;i++)
                for(int j=0;j<3;j++)
                    v.push_back(begNode.maze[i][j]);
            int sum=0;
            for(int i=0;i<9;i++)
                for(int j=i+1;j<9;j++)
                    if(v[j]&&v[i]&&v[i]>v[j])
                        sum++;
            return !(sum&1);

        }
        void aStar()
        {
            if(begNode.Hash==dest)
                return ;
            std::priority_queue<Node> que;
            que.push(begNode);
            while(!que.empty())
            {
                Node u=que.top();
                que.pop();
                for(int i=0;i<4;i++)
                {
                    Node v=u;
                    v.pos_x+=way[i][0];
                    v.pos_y+=way[i][1];
                    if(v.check())
                    {
                        std::swap(v.maze[v.pos_x][v.pos_y],v.maze[u.pos_x][u.pos_y]);
                        v.Hash=getHash(v);
                        if(vis[v.Hash]==-1)
                        {
                            vis[v.Hash]=i;
                            v.fun_g++;
                            pre[v.Hash]=u.Hash;
                            v.fun_h=getH(v);
                            que.push(v);
                        }
                        if(v.Hash==dest)
                            return ;
                    }
                }
            }

        }
    private:
        Node begNode;
        int getHash(Node &tmp)
        {
            std::vector<int> v;
            for(int i=0;i<3;i++)
                for(int j=0;j<3;j++)
                    v.push_back(tmp.maze[i][j]);
            int res=0;
            for(int i=0;i<9;i++)
            {
                int k=0;
                for(int j=i+1;j<9;j++)
                    if(v[j]<v[i])
                        k++;
                res+=HASH[8-i]*k;
            }
            return res;
        }


        int getH(Node &tmp)
        {
            int ans=0;
            for(int i=0;i<3;i++)
                for(int j=0;j<3;j++)
                    if(tmp.maze[i][j])
                        ans+=abs(i-(tmp.maze[i][j]-1)/3)+abs(j-(tmp.maze[i][j]-1)%3);
            return ans;
        }
};


string solveEight(const StringPiece& puzzle)
{
    string board="";
    string constStr="No solution";
    for(int i=0;i<puzzle.size();i++)
    {
        board+=puzzle[i];
    }
    EightSolver s(board);
    if(!s.isOK())
        return constStr;
    else
    {
       s.aStar();
       return s.getAns();
    }

}


