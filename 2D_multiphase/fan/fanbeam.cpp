//reconstructing a binary materials in 2D
//from a number of projections at different angles

//the attenuation coefficient is taken to be either 0 or 1
//the "projection" simply is proportaional to the attenuation, instead of (1 - attenuation), etc.
//we assume fan beam geomery and consider that the incoming ray is sufficiently dense, so every pixel will have a projection. The resolution is completeled determined by the detector then...


using namespace std;

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXS 100000 //a module for random number

#define MAXX 193 //system size
#define MAXY 202

#define Lmax 250 //determing the receptor resolution, the unit is one pixel
                 //the total number of bins in the recepotr 2*Lmax+1
#define L_1 400  //distance between the x-ray source and the center of the specimen.
#define L_2 200  //distance between the center of the specimen and the detector.

#define N_p 30   //number of projections, which are assumed to be evenly distributed over pi   

double mu_1 = 1.0; //attenuation coefficient of phase 1
double mu_2 = 0.0; //attenuation coefficient of phase 2

int N_tot = 0; //the total number of black pixels...

double config[MAXX][MAXY]; //a binary configuration with 0 and 1
//this is used for both storing the initial configuraion 
// and used for the recontruction after the projection is collected...
double projection[N_p+1][2*Lmax+1]; //the set of projections. 
                                  //fan beam projection length equal to (2*Lmax+1) for test....


//double final_config[MAXX][MAXY]; //the final reconstructed configuration
double temp_projection[N_p+1][2*Lmax+1]; //this is the projection profile associated with an intermeidate configuration....
int changed_location[N_p+1][2]; //the projected location of the changed pixel
double changed_projection[N_p+1][2]; //the associated value due to the changed pixel

double x_c = (double)MAXX/2.0;
double y_c = (double)MAXY/2.0;

double d_theta = 180/(double)N_p;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//for simulated annealing reconstruction...

int indexi; int indexj; int indexm; int indexn;

//The cooling schedule ...
int Nevl = 1000000; //maximum trials at a T-stage

double alpha = 0.98; //cooling rate

int TN = 180; //total T-stages
              //maximum accepted trial moves at a T-stage

double T = 0.01; //initial temperature

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void read_config()
{
  double sum = 0.0;
  double value;

  ifstream fin;

  fin.open("target_config.txt");

  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
	fin>>value;

	config[i][j] = value;

	sum += value;
      }

  sum = sum/(MAXX*MAXY);

  int counter = 0;

  fin.close();

  //now binarize the configuration 
  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
	if(config[i][j]>= sum)
	  {
	    config[i][j] = 1;

	    counter++;
	  }
	else 
	  config[i][j] = 0;
      }


  cout<<"N_tot = "<<counter<<endl;

  N_tot = counter;
}

double dist(int temp_x, int temp_y) //distance from the select point to the center
{
  double temp_dist2 = (temp_x - x_c)*(temp_x - x_c) + (temp_y - y_c)*(temp_y - y_c);
  
  return sqrt(temp_dist2);
}

double fanprojection(int temp_l, int temp_bin)
{
  return (temp_l * (L_1 + L_2))/(L_1 + temp_bin);
}

double get_angle(int temp_x, int temp_y)
{
  double val = (double)(temp_y - y_c)/(double)(temp_x - x_c);

  double temp_theta = atan(val);

  if((temp_x-x_c)>=0) return (temp_theta);
  else if((temp_x-x_c)<0) return (temp_theta+180);     
 
}

void get_projections()
{
  double temp_l; //this is the distance on the detector to the projection of the rotation center
  int temp_bin;
  int temp_pro;

  //loop over every angle...
  for(int i=0; i<N_p+1; i++)
    {
      //cout<<i<<endl;

      //now check each point in the phase...
      for(int m=0; m<MAXX; m++)
	for(int n=0; n<MAXY; n++)
	  {
	    //cout<<m<<" "<<n<<endl;
	    
	    temp_l = dist(m, n)*cos(get_angle(m, n) + i*d_theta);
	    temp_bin = dist(m,n)*sin(get_angle(m, n) + i*d_theta);

	    temp_pro = (int)floor(fanprojection(temp_l, temp_bin));

	    //cout<<temp_bin<<endl;

	    if(config[m][n] == 1)
	      {
		if(temp_pro<(2*Lmax+1) && temp_pro>=0)
		  projection[i][temp_pro] += mu_1;
	      }
	    else
	      {
		if(temp_pro<(2*Lmax+1) && temp_pro>=0)
		  projection[i][temp_pro] += mu_2;
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

  for(int i=0; i<N_p+1; i++)
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

  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
	if(config[i][j] == 1)
	  {
	    fout2<<i<<"  "<<j<<endl;
	  }
      }

  fout2.close();
}


//given the position and angle
//get the projection of the point on the detector...
int get_point_projection(int temp_x, int temp_y, double temp_theta)
{
  double temp_l; 
  int temp_bin;
  int temp_pro;

  temp_l = dist(temp_x, temp_y)*cos(get_angle(temp_x, temp_y) + temp_theta);

  temp_bin = dist(temp_x, temp_y)*sin(get_angle(temp_x, temp_y) + temp_theta); 

  temp_pro = (int)floor(fanprojection(temp_l, temp_bin));
  
  return temp_pro;
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
        config[i][j] = 0;
      }

  
  for(int i=0; i<N_tot; i++)
    {
      int m = rand()%MAXX;
      int n = rand()%MAXY;

      while(config[m][n]==1)
	{
	  m = rand()%MAXX;
	  n = rand()%MAXY;
	}

      config[m][n] = 1;
    }

  double temp_theta;
  int temp_pro;
  //now, initialize the projections..
  for(int i=0; i<N_p+1; i++)
    {

      for(int j=0; j<2*Lmax+1; j++)
	temp_projection[i][j] = 0;

      temp_theta = i*d_theta;

      //now check each point in the phase...
      for(int m=0; m<MAXX; m++)
	for(int n=0; n<MAXY; n++)
	  {
	    temp_pro = get_point_projection(m, n, temp_theta);
	    
	    if(config[m][n] == 1)
	      {
		if(temp_pro<(2*Lmax+1) && temp_pro>=0)
		  temp_projection[i][temp_pro] += mu_1;
	      }
	    else
	      {
		if(temp_pro<(2*Lmax+1) && temp_pro>=0)
		  temp_projection[i][temp_pro] += mu_2;
	      }
	     
	  }
      
    }

}

void change_config()
{
  int i, j, m, n;

  //fist we change the lines
 
  do{   i = rand()%MAXX;
        m = rand()%MAXX;
 
        j = rand()%MAXY; 
        n = rand()%MAXY; 

    }while(config[i][j] == config[m][n]);

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
  int temp_pro1, temp_pro2;

  double temp_theta; 
  
  //there are the rare event that the two selected pixel have the same value...
  if(config[indexi][indexj] != config[indexm][indexn])
    {
      for(int i=0; i<N_p+1; i++)
	{
	  temp_theta = i*d_theta;
	  
	  temp_pro1 = get_point_projection(indexi, indexj, temp_theta);
	   
	  temp_pro2 = get_point_projection(indexm, indexn, temp_theta);
	  
	  changed_location[i][0] = temp_pro1;
	  changed_location[i][1] = temp_pro2;
	  
	  //this means the old pixel is 0
	  if(config[indexi][indexj] == 1)
	    {
	      changed_projection[i][0] = mu_1 - mu_2;
	      changed_projection[i][1] = mu_2 - mu_1;
	    }
	  else
	    {
	      changed_projection[i][0] = mu_2 - mu_1;
	      changed_projection[i][1] = mu_1 - mu_2;
	    }

	  
	}
    }
  else //nothing needs to be changed ...
    {
        for(int i=0; i<N_p+1; i++)
	  for(int j=0; j<2; j++)
	    {
	      changed_location[i][j] = 0;
	      changed_projection[i][j] = 0;
	    }
      
    }
}


void update_temp_projection()
{

  int temp_pro1, temp_pro2;

  for(int i=0; i<N_p+1; i++)
    {
      temp_pro1 = changed_location[i][0];
      temp_pro2 = changed_location[i][1];

      if(temp_pro1<(2*Lmax+1) && temp_pro1>=0)
	temp_projection[i][temp_pro1] = temp_projection[i][temp_pro1] + changed_projection[i][0];

      if(temp_pro2<(2*Lmax+1) && temp_pro2>=0)
	temp_projection[i][temp_pro2] = temp_projection[i][temp_pro2] + changed_projection[i][1];
    }
}


void resume_temp_projection()
{
  int temp_pro1, temp_pro2;

  for(int i=0; i<N_p+1; i++)
    {
      temp_pro1 = changed_location[i][0];
      temp_pro2 = changed_location[i][1];

      if(temp_pro1<(2*Lmax+1) && temp_pro1>=0)
	temp_projection[i][temp_pro1] = temp_projection[i][temp_pro1] - changed_projection[i][0];

      if(temp_pro2<(2*Lmax+1) && temp_pro2>=0)
	temp_projection[i][temp_pro2] = temp_projection[i][temp_pro2] - changed_projection[i][1];
    }

}

//no way to around this, has to be a sum between target and sampled quantities...
//this is normalized per projection and per bin
double get_energy()
{
  double sum = 0;

  for(int i=0; i<N_p+1; i++)
    {
      for(int j=0; j<(2*Lmax+1); j++)
	sum += (projection[i][j] - temp_projection[i][j])*(projection[i][j] - temp_projection[i][j]);
    }

  return sum/(N_p*(2*Lmax+1));
}


double PE(double dE, double T)
{
  if(dE > 0) return exp(-dE/T);
  else return 1;
}


void print_final_config()
{
  ofstream fout2;
  
  fout2.open("final_config.txt");

  for(int i=0; i<MAXX; i++)
    for(int j=0; j<MAXY; j++)
      {
	if(config[i][j] == 1)
	  {
	    fout2<<i<<"  "<<j<<endl;
	  }
      }

  fout2.close();
  
}


void print_final_projections()
{
  ofstream fout2;

  fout2.open("final_projection.txt");

  cout<<"printing projections now..."<<endl;

  for(int i=0; i<N_p+1; i++)
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
  for(int i=1; i<MAXX; i++)
    for(int j=1; j<MAXY; j++)
      {
	if(config[i][j]==1 && config[i-1][j]==0 && config[i+1][j]==0 && config[i][j-1]==0 && config[i][j+1]==0)
	  config[i][j]=0;
      }

  print_final_config();

  print_final_projections();
  
}



