#include<bits/stdc++.h>
using namespace std;

pair<int,int> indexsofgivenSum(vector<int>&arr,int target){

int n=arr.size();
int l=0,r=0;
int sum=0;

while(r<n){
	sum+=arr[r];
	
	while(sum>target){
		sum-=arr[l];
		l++;
	}
	
	if(sum==target){
		return {l,r};
	}
	r++;
}

return {-1,-1};
}
		

//2        q-2

//4 5	q-4,5


//3 7 6 8 q-3,7,6,8

struct TreeNode{
public:
	TreeNode* left;
	TreeNode* right;
	int data;
	TreeNode(){
		left=NULL;
		right=NULL;
	}
};
vector<vector<int>> LevelOrderTraversal(TreeNode* root){
	vector<vector<int>>ans;
	if(root==NULL)return ans;
	TreeNode* temp=root;
	queue<TreeNode*>q;
	q.push(temp);


	while(!q.empty()){
		bool leftToright=true;
		int size=q.size();
		vector<int>level(size);
		for(int i=0;i<size;i++){
			TreeNode* frontnode=q.front();
			q.pop();
			int index=leftToright?i:size-1-i;
			level[index]=frontnode->data;
			if(frontnode->left)q.push(frontnode->left);
			if(frontnode->right)q.push(frontnode->right);
			
		}
		leftToright=!leftToright;
		ans.push_back(level);
	}
	return ans;
}


