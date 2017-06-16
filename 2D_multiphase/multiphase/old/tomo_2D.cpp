//reconstructing a binary materials in 2D
//from a number of projections at different at different angles

//the attenuation coefficient is taken to be either 0 or 1
//the "projection" simply is proportaional to the attenuation, instead of (1 - attenuation), etc.
//we assume parallel beam geomery and consider that the incoming ray is sufficiently dense, so every pixel will have a projection. The resolution is completeled determined by the detector then...


using namespace std;

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXS 100000 //a module for random number

#define MAXX 169 //system size
#define MAXY 147
#define Lmax 112 //determing the receptor resolution, the unit is one pixel
               //the total number of bins in the recepotr 2*Lmax+1
#define N_p 40 //number of projections, which are assumed to be evenly distributed over pi  
 

double mu_1 = 1.0; //attenuation coefficient of phase 1
double mu_2 = 0.5; //attenuation coefficient of phase 2
double mu_3 = 0.0; //attenuation coefficient of phase 3

double counter_1;
double counter_2;


double config[MAXX][MAXY]; //a binary configuration with 0 and 1
//this is used for both storing the initial configuraion 
// and used for the recontruction after the projection is collected...
double projection[N_p][2*Lmax+1]; //the set of projections.


//double final_config[MAXX][MAXY]; //the final reconstructed configuration
double temp_projection[N_p][2*Lmax+1]; //this is the projection profile associated with an intermeidate configuration....
int changed_location[N_p][2]; //the projected location of the changed pixel
double changed_projection[N_p][2]; //the associated value due to the changed pixel

//ofstream fout;
//ifstream fin;

double x_c = (double)MAXX/2.0;
double y_c = (double)MAXY/2.0;

double d_theta = 180/(double)N_p;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//for simulated annealing reconstruction...

int indexi; int indexj; int indexm; int indexn;

//The cooling schedule ...
int Nevl = 5000000; //maximum trials at a T-stage

double alpha = 0.98; //cooling rate

int TN = 500; //total T-stages

double T = 0.05; //initial temperature

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void read_config()
{
  double value;

  ifstream fin;

  fin.open("metal_ceramic.txt");

  for(int j=0; j<MAXY; j++)
    for(int i=0; i<MAXX; i++)
      {
	fin>>value;

	config[i][j] = value;
      }

  fin.close();

  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
	if(config[i][j] < 50 && config[i][j] >= 0)
	  {
	    config[i][j] = 0;

	    counter_1++;
	  }
	else if(config[i][j] < 100 && config[i][j] >= 50)
	  {
	    config[i][j] = 50;

	    counter_2++;
	  }
	else
	  config[i][j] = 255;
      }

  //get rid of those pixels surrounded by other phases
  /* 
  for(int i=1; i<MAXX-1; i++)
    for(int j=1; j<MAXY-1; j++)
      {
	if(config[i][j] !=0 && config[i-1][j] == 0 && config[i+1][j] == 0 && config[i][j-1] == 0 && config[i][j+1] == 0)
	  {
	    config[i][j] = 0;

	    counter_1++;
	  }
	else if(config[i][j] != 50 && config[i-1][j] == 50 && config[i+1][j] == 50 && config[i][j-1] == 50 && config[i][j+1] == 50)
	  {
	    config[i][j] = 50;

	    counter_2++;
	  }
	else if(config[i][j] != 255 && config[i-1][j] == 255 && config[i+1][j] == 255 && config[i][j-1] == 255 && config[i][j+1] == 255)
	  config[i][j] = 255;
      }
  */

}

double dist(int temp_x, int temp_y) //distance from the select point to the center
{
  double temp_dist2 = (temp_x - x_c)*(temp_x - x_c) + (temp_y - y_c)*(temp_y - y_c);
  
  return sqrt(temp_dist2);
}

double get_angle(int temp_x, int temp_y)
{
  double val = (double)(temp_y - y_c)/(double)(temp_x - x_c);

  double temp_theta = atan(val);

  if((temp_x-x_c)>=0) return (temp_theta);
  else if((temp_x-x_c)<=0) return (temp_theta+180);     
 
}

void get_projections()
{
  double temp_l; //this is the distance on the detector to the projection of the rotation center
  int temp_bin;

  //loop over every angle...
  for(int i=0; i<N_p; i++)
    {
      //cout<<i<<endl;

      //now check each point in the phase...
      for(int m=0; m<MAXX; m++)
	for(int n=0; n<MAXY; n++)
	  {
	    //cout<<m<<" "<<n<<endl;
	    
	    temp_l = dist(m, n)*cos(get_angle(m, n) + i*d_theta); 
	    
	    temp_bin = (int)floor(temp_l+Lmax);

	    //cout<<temp_bin<<endl;

	    if(config[m][n] == 0)
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  projection[i][temp_bin] += mu_1;
	      }
	    else if(config[m][n] == 50)
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  projection[i][temp_bin] += mu_2;
	      }
	    else
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  projection[i][temp_bin] += mu_3;
	      }
	  }
    }

  cout<<"finish computing the projections..."<<endl;  //get the tomography projections for the sample
}



void print_projections()
{
  ofstream fout2;

  fout2.open("projection.txt");

  cout<<"printing projections now..."<<endl;

  for(int i=0; i<N_p; i++)
    {
      //cout<<i<<endl;

      fout2<<"### angle = "<<i*d_theta<<endl;

      for(int r=0; r<(2*Lmax+1); r++)
	{
	  fout2<<(r-Lmax)<<" "<<projection[i][r]<<endl;
	}

      fout2<<"#############################"<<endl;
      fout2<<endl<<endl<<endl;
    }

  fout2.close();
}

void print_config()
{
  ofstream fout2;

  fout2.open("binary_config.txt");

  for(int j=0; j<MAXY; j++)
    for(int i=0; i<MAXX; i++)
      {
	fout2<<config[i][j]<<endl;
      }

  fout2.close();
}


//given the position and angle
//get the projection of the point on the detector...
int get_point_projection(int temp_x, int temp_y, double temp_theta)
{
  double temp_l; 
  int temp_bin;

  temp_l = dist(temp_x, temp_y)*cos(get_angle(temp_x, temp_y) + temp_theta);

  temp_bin = (int)floor(temp_l+Lmax); 
  
  return temp_bin;
}

//initilaize the initial configuration from the total number of black pixels
//it is assumed that this information can be obtained from the sum of the projection devivided by mu
//since now the only available data is considered to the projection, config is used for recosntruction
void init_reconstr()
{
  //first, clear whatever in the list...
  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
        config[i][j] = 255;
      }
 
  for(int i=0; i<counter_1; i++)
    {
      int m = rand()%MAXX;
      int n = rand()%MAXY;

      while(config[m][n]==0)
	{
	  m = rand()%MAXX;
	  n = rand()%MAXY;
	}

      config[m][n] = 0;
    }

  for(int i=0; i<counter_2; i++)
    {
      int m = rand()%MAXX;
      int n = rand()%MAXY;

      while(config[m][n]==0 || config[m][n]==50)
	{
	  m = rand()%MAXX;
	  n = rand()%MAXY;
	}

      config[m][n] = 50;
    }


  double temp_theta;
  int temp_bin;
  //now, initialize the projections..
  for(int i=0; i<N_p; i++)
    {

      for(int j=0; j<(2*Lmax+1); j++)
	temp_projection[i][j] = 0;


      temp_theta = i*d_theta;

      //now check each point in the phase...
      for(int m=0; m<MAXX; m++)
	for(int n=0; n<MAXY; n++)
	  {
	    temp_bin = get_point_projection(m, n, temp_theta);
	    
	    if(config[m][n] == 0)
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  temp_projection[i][temp_bin] += mu_1;
	      }
	    else if(config[m][n] == 50)
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  temp_projection[i][temp_bin] += mu_2;
	      }
	    else
	      {
		if(temp_bin<(2*Lmax+1) && temp_bin>=0)
		  temp_projection[i][temp_bin] += mu_3;
	      }
	  }
      
    }

}

void change_config()
{
  int i, j, m, n;

  srand(time(NULL));

  i = rand()%MAXX;
  m = rand()%MAXX;
 
  j = rand()%MAXY; 
  n = rand()%MAXY; 
 
  while(config[i][j] == config[m][n])
    {
      i = rand()%MAXX;
      j = rand()%MAXY;

      m = rand()%MAXX;
      n = rand()%MAXY;
    }

  double temp;
  
  temp = config[i][j];
  config[i][j] = config[m][n];
  config[m][n] = temp;
  
  indexi = i;
  indexj = j;
  indexm = m;
  indexn = n;

}


void resume_config()
{
  double temp;
  //first we resume the config
  temp = config[indexi][indexj];
  config[indexi][indexj] = config[indexm][indexn];
  config[indexm][indexn] = temp;

}


void get_projection_difference()
{
  int temp_bin1, temp_bin2;
  double temp_theta; 
  
  //there are the rare event that the two selected pixel have the same value...
  if(config[indexi][indexj] != config[indexm][indexn])
    {
      for(int i=0; i<N_p; i++)
	{
	  temp_theta = i*d_theta;
	  
	  temp_bin1 = get_point_projection(indexi, indexj, temp_theta);
	  
	  temp_bin2 = get_point_projection(indexm, indexn, temp_theta);
	  
	  changed_location[i][0] = temp_bin1;
	  changed_location[i][1] = temp_bin2;
	  
	  //this means the old pixel is 0
	  if(config[indexi][indexj] == 0 && config[indexm][indexn] == 50)
	    {
	      changed_projection[i][0] = mu_1 - mu_2;
	      changed_projection[i][1] = mu_2 - mu_1;
	    }
	  else if(config[indexi][indexj] == 50 && config[indexm][indexn] == 0)
	    {
	      changed_projection[i][0] = mu_2 - mu_1;
	      changed_projection[i][1] = mu_1 - mu_2;
	    }
	  else if(config[indexi][indexj] == 0 && config[indexm][indexn] == 255)
	    {
	      changed_projection[i][0] = mu_1 - mu_3;
	      changed_projection[i][1] = mu_3 - mu_1;
	    }
	  else if(config[indexi][indexj] == 255 && config[indexm][indexn] == 0)
	    {
	      changed_projection[i][0] = mu_3 - mu_1;
	      changed_projection[i][1] = mu_1 - mu_3;
	    }
	  else if(config[indexi][indexj] == 50 && config[indexm][indexn] == 255)
	    {
	      changed_projection[i][0] = mu_2 - mu_3;
	      changed_projection[i][1] = mu_3 - mu_2;
	    }
	  else if(config[indexi][indexj] == 255 && config[indexm][indexn] == 50)
	    {
	      changed_projection[i][0] = mu_3 - mu_2;
	      changed_projection[i][1] = mu_2 - mu_3;
	    }
	  
	}
    }
  else //nothing needs to be changed ...
    {
        for(int i=0; i<N_p; i++)
	  for(int j=0; j<2; j++)
	    {
	      changed_location[i][j] = 0;
	      changed_projection[i][j] = 0;
	    }
      
    }
}


void update_temp_projection()
{

  int temp_bin1, temp_bin2;

  for(int i=0; i<N_p; i++)
    {
      temp_bin1 = changed_location[i][0];
      temp_bin2 = changed_location[i][1];

      if(temp_bin1<(2*Lmax+1) && temp_bin1>=0)
	temp_projection[i][temp_bin1] = temp_projection[i][temp_bin1] + changed_projection[i][0];

      if(temp_bin2<(2*Lmax+1) && temp_bin2>=0)
	temp_projection[i][temp_bin2] = temp_projection[i][temp_bin2] + changed_projection[i][1];
    }
}


void resume_temp_projection()
{
  int temp_bin1, temp_bin2;

  for(int i=0; i<N_p; i++)
    {
      temp_bin1 = changed_location[i][0];
      temp_bin2 = changed_location[i][1];

      if(temp_bin1<(2*Lmax+1) && temp_bin1>=0)
	temp_projection[i][temp_bin1] = temp_projection[i][temp_bin1] - changed_projection[i][0];

      if(temp_bin2<(2*Lmax+1) && temp_bin2>=0)
	temp_projection[i][temp_bin2] = temp_projection[i][temp_bin2] - changed_projection[i][1];
    }

}

//no way to around this, has to be a sum between target and sampled quantities...
//this is normalized per projection and per bin
double get_energy()
{
  double sum = 0;

  for(int i=0; i<N_p; i++)
    {
      for(int j=0; j<(2*Lmax+1); j++)
	sum += (projection[i][j] - temp_projection[i][j])*(projection[i][j] - temp_projection[i][j]);
    }

  return sum/(N_p*(2*Lmax+1));
}


double PE(double dE, double T) // the probability that should compare ...
{
  if(dE > 0) return exp(-dE/T);
  else return 1;
}


void print_final_config()
{
  ofstream fout2;
  
  fout2.open("final_config.txt");

  for(int j=0; j<MAXY; j++)
    for(int i=0; i<MAXX; i++)
      {
	    fout2<<config[i][j]<<endl;
      }

  fout2.close();
  
}


void print_final_projections()
{
  ofstream fout2;

  fout2.open("final_projection.txt");

  cout<<"printing projections now..."<<endl;

  for(int i=0; i<N_p; i++)
    {
      //cout<<i<<endl;

      fout2<<"### angle = "<<i*d_theta<<endl;

      for(int r=0; r<(2*Lmax+1); r++)
	{
	  fout2<<(r-Lmax)<<" "<<temp_projection[i][r]<<endl;
	}

      fout2<<"#############################"<<endl;
      fout2<<endl<<endl<<endl;
    }

  fout2.close();
}

int main()
{
  //sample the projection from an input image...

  srand(time(NULL));
  
  read_config();

  cout<<"here"<<endl;

  get_projections();

  cout<<"here"<<endl;

  print_projections();

  print_config();

  //reconstruct an image from the sampled projections...
  //**********************************************************

  init_reconstr();

  int N_acc = 0; //acceptance rate...

  double E_old, E_new;
  double E_t;
  double energyb = 10000000000.0;

  //saving the energy of each stage...
  FILE* fp = fopen("E.txt","w");
  fclose(fp);

  cout<<"Staring the simulated annealing reconstruction process..."<<endl;
  for(int q=0; q<TN; q++)
    {
      T = alpha*T;

      N_acc = 0;

      cout<<"Stage "<<q+1<<" with T = "<<T<<endl;
       
      for(int i=0; i< Nevl; i++)
	{
	  //cout<<"change "<<i<<endl;

	  change_config();
	  //sample S2 for the new configuration, using time saving methods
	   
	  get_projection_difference();
		  
	  E_old =  get_energy();
	  
	  update_temp_projection();
	  
	  E_new = get_energy();
	  
	  //cout<<"dE = "<<(E_new-E_old)<<endl;
	  
	  double P = (double)(rand()%MAXS)/(double)MAXS;
	  
	  if( P > PE((E_new-E_old), T))
	    {
	      resume_config();
	      //this just resumes the 'configuration', still need to resume S2...
	      
	      resume_temp_projection();
	      
	      E_t = E_old;
	      
	    } 
	  else 
	    {
	      N_acc ++;
	      
	      E_t = E_new;
	    }
	  
	  
	  //compare and record the best energy and configuration... 
	  
	  if(E_t < energyb)
	    {
	      energyb = E_t;
	      
	    }
	  
	   //printf("%f   %d change has finished... \n", energyt, i+1 );
	  
	}
      
      printf("%d th change of temperature has finished... \n",q+1 );
      cout<<"The acceptance rate: "<<(double)N_acc/(double)Nevl<<endl;
      cout<<"The energy E = "<<energyb<<endl;
      
      
      fp = fopen("E.txt","a");
      fprintf(fp, "%e\n", energyb);
      fclose(fp);
       
       
       
      printf("*************************************************\n");
       
    }
  
   //*****************************************************************
   //*****************************************************************
   //this is the end of simulated annealing
   

  print_final_config();

  print_final_projections();
  
}



