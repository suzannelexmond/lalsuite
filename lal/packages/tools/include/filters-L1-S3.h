/* Inverse of sensing function */

#define CinvDirectOrder 13
const REAL8   CinvDirectCoefs[CinvDirectOrder]={
  6.319293379185171e-11,    -6.433074462017784e-10,     2.999733408071718e-09,    
 -8.470507012948007e-09,     1.612862619174526e-08,    -2.181161514646775e-08,
  2.147703740846217e-08,    -1.551092582707976e-08,     8.152605158864128e-09,    
 -3.040582316047673e-09,     7.636229464913452e-10,    -1.159226632921570e-10,
  8.042364615343168e-12};

#define CinvRecursOrder 13
const REAL8   CinvRecursCoefs[CinvRecursOrder]={
  1.000000000000000e+00,    -5.702630135108517e+00,     1.503360844219251e+01,    
 -2.408247074633077e+01,     2.569548346547491e+01,    -1.832488461558141e+01,
  7.852784685163898e+00,    -1.177946818785043e+00,    -4.294349994751644e-01,     
  1.087647421360701e-01,     3.122293153962438e-02,     2.468552694941967e-03,
  6.460281790651390e-05};


/*************** Servo -- 9 digital filters ********************/

#define ServoGain -5.835900783201030e-01

#define G_Dord 3 
#define G_Rord 3 
#define NGfilt 9

const REAL8 G_D[NGfilt][G_Dord]={
{1.0000000000000000e+00, -1.9906866972471560e+00, 9.9079513186864243e-01},
{1.0000000000000000e+00, -1.9622400267004301e+00, 9.6236990677318002e-01},
{1.0000000000000000e+00, 2.0000000000000000e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, -1.9937328610916600e+00, 9.9388299711621997e-01},
{1.0000000000000000e+00, -9.9426405403630003e-01, 0.0000000000000000e+00},
{1.0000000000000000e+00, -1.9996724404853401e+00, 9.9969319369916998e-01},
{1.0000000000000000e+00, -1.9995833272183801e+00, 9.9961586209818998e-01},
{1.0000000000000000e+00, -1.9995061547329900e+00, 9.9955067104336004e-01},
{1.0000000000000000e+00, -1.9994851270591900e+00, 9.9949077893281002e-01}
};


const REAL8 G_R[NGfilt][G_Rord]={
{1.0000000000000000e+00, -1.9987796692903781e+00, 9.9878004159213785e-01},
{1.0000000000000000e+00, -1.4663200350108301e+00, 6.3657941182889999e-01},
{1.0000000000000000e+00, -1.0300153394928500e+00, 4.3056923506806000e-01},
{1.0000000000000000e+00, -1.9976540961780000e+00, 9.9770169124178998e-01},
{1.0000000000000000e+00, -9.9961657831852002e-01, 0.0000000000000000e+00},
{1.0000000000000000e+00, -1.9999485590540000e+00, 9.9996931513347997e-01},
{1.0000000000000000e+00, -1.9999290390635500e+00, 9.9996157956838005e-01},
{1.0000000000000000e+00, -1.9999105327038500e+00, 9.9995505801716000e-01},
{1.0000000000000000e+00, -1.9998664121082099e+00, 9.9987206505960002e-01}};


/***************** Actuation function ***************/

/* analog part: 14th order filter */



#define A_0_Dord 15

const REAL8 A_0_D[A_0_Dord] = {
  -6.871448454166455e-24,    -1.074737071914077e-22,    -9.926460542956044e-22,    -5.702570992692499e-21,    
  -2.218930705356857e-20,    -6.245971644319258e-20,    -1.315616393211633e-19,    -2.098242117119467e-19,    
  -2.531341240403217e-19,    -2.287395385087259e-19,    -1.518422914198092e-19,    -7.158296379681802e-20,
  -2.257659922286659e-20,    -4.242293382248136e-21,    -3.552899823361631e-22};

#define A_0_Rord 15
const REAL8 A_0_R[A_0_Rord] = { 
   1.000000000000000e+00,     9.950357471430785e-01,    -1.285040610507381e-01,    -1.027956835072690e+00,  
  -1.490676963902109e+00,    -6.446395792428853e-01,     2.007401263944038e-02,     1.688835516888857e-01,     
   3.366290468545587e-01,     3.711869727877520e-01,     2.470428965350086e-01,     1.161260756270791e-01,
   3.204586291659955e-02,     4.347991487845589e-03,     4.076690548826582e-04};

/* Digital part: 7 2nd order filters the x-arm; 6 2nd order filters for the y-arm */

#define A_digital_Dord 3 
#define A_digital_Rord 3 

#define NAXfilt 11
#define NAYfilt 11

#define AXGain 9.916806430054919e-01
#define AYGain 9.831763931851054e-01

const REAL8 AX_D[NAXfilt][A_digital_Dord]={
{1.0000000000000000e+00, -1.9308621283672200e+00, 9.9994788243216004e-01},
{1.0000000000000000e+00, 1.8356693172828000e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8357465513258600e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8358576131384201e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8359347624693600e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6593733368616499e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6594287344114500e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6595272917571400e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6596426100422801e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6597411112497700e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6597964545475301e+00, 1.0000000000000000e+00}};

const REAL8 AX_R[NAXfilt][A_digital_Rord]={
{1.0000000000000000e+00, -1.9258936701136999e+00, 9.9480165401062004e-01},
{1.0000000000000000e+00, 1.8332524948728799e+00, 9.9818089208225003e-01},
{1.0000000000000000e+00, 1.8350161963119200e+00, 9.9819017469388005e-01},
{1.0000000000000000e+00, 1.8350005303119401e+00, 9.9978875967223002e-01},
{1.0000000000000000e+00, 1.8362143960696500e+00, 9.9978950523321997e-01},
{1.0000000000000000e+00, 1.6576531144475899e+00, 9.9825336491595995e-01},
{1.0000000000000000e+00, 1.6586193482420399e+00, 9.9825560741428998e-01},
{1.0000000000000000e+00, 1.6583008661157901e+00, 9.9931646565751997e-01},
{1.0000000000000000e+00, 1.6587680988889599e+00, 9.9983023874550003e-01},
{1.0000000000000000e+00, 1.6597343758398400e+00, 9.9931776877051004e-01},
{1.0000000000000000e+00, 1.6601191167356599e+00, 9.9983054424735995e-01}};


const REAL8 AY_D[NAYfilt][A_digital_Dord]={
{1.0000000000000000e+00, -1.9308621283672200e+00, 9.9994788243216004e-01},
{1.0000000000000000e+00, 1.8356693172828000e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8357465513258600e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8358576131384201e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.8359347624693600e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6557353653059199e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6559755943807200e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6564026963567200e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6569019647809300e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6573280244756501e+00, 1.0000000000000000e+00},
{1.0000000000000000e+00, 1.6575672462886599e+00, 1.0000000000000000e+00}};


const REAL8 AY_R[NAYfilt][A_digital_Rord]={
{1.0000000000000000e+00, -1.9258936701136999e+00, 9.9480165401062004e-01},
{1.0000000000000000e+00, 1.8332524948728799e+00, 9.9818089208225003e-01},
{1.0000000000000000e+00, 1.8350161963119200e+00, 9.9819017469388005e-01},
{1.0000000000000000e+00, 1.8350005303119401e+00, 9.9978875967223002e-01},
{1.0000000000000000e+00, 1.8362143960696500e+00, 9.9978950523321997e-01},
{1.0000000000000000e+00, 1.6483304545812101e+00, 9.9247326413893999e-01},
{1.0000000000000000e+00, 1.6525279061980600e+00, 9.9251449886589005e-01},
{1.0000000000000000e+00, 1.6510957460767399e+00, 9.9704627036015003e-01},
{1.0000000000000000e+00, 1.6531098360034699e+00, 9.9926592212304999e-01},
{1.0000000000000000e+00, 1.6573103760081600e+00, 9.9707031548605995e-01},
{1.0000000000000000e+00, 1.6589609993737999e+00, 9.9927156334375999e-01}};




