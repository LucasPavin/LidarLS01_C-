#define CBR_230400 230400    ///Rajouté pour pouvoir
///configurer une nouvelle vitesse en bauds dans serialib.cpp
/********Include Masqués********/
//#include <unistd.h>
//#include <windows.h>
/*********Include********/
#include <iostream>
#include <string>
#include <stdio.h>
#include <thread>    /// Initialisation de la librairie pour pouvoir utiliser les threads
#include "serialib.h"
#include <fstream>   ///Utilisé dans fichierConfiguration
#include <map>

/**********DEFINE*********/
/*#define SERIAL_PORT "COM5"   /// Definition du port série
#define PAS_ANGLE 1*/

using namespace std;
/**********CLASSE*************/
void supprimerEspaces(string &s)   ///Si on modifie S on modifie le paramètre réel passé
{
    while (s[0] == ' ')  /// Suppression des espaces en début de chaine
        s = s.substr(1);   ///On met dans S la sous chaîne de S commençant au second caractères
    while (s[s.length()-1] == ' ')  /// Suppression des espaces en fin de chaine
        s = s.substr(0, s.length()-1); /// On met dans S la sous chaîne de S, commençant au premier
                                       ///caractère et privée de son dernier caractère
}
class FichierConfiguration
{
protected :
    map<string, string> lignes;
    bool ok;
public :
    FichierConfiguration(string nomFichier)
    {
        if (nomFichier=="")
        {
            ok = false;
            return;
        }
        ok = true;
        ifstream i(nomFichier); ///
        if (! i)
        {
            ok = false;
            return;
        }

        string ligne;
        while (getline(i, ligne))
        {
            size_t pos = ligne.find("=");
            if (pos!=string::npos)
            {
                string cle = ligne.substr(0, pos);
                supprimerEspaces(cle);
                string valeur = ligne.substr(pos+1);
                supprimerEspaces(valeur);
                lignes.insert(make_pair(cle, valeur));
            }
        }
        i.close();
        ok = true;
    }
    bool getOk()
    {
        return this->ok;
    }
    string lireValeur(string cle)
    {
        string valeur;
        try
        {
            valeur = lignes.at(cle);
        }
        catch(exception e)
        {
            valeur="";
        }
        return valeur;
    }
};
class ParametresRS232
{
protected:
    int vitBau;
    string nomPortCOM;
    int taiDon;
    int nbrBitSto;
public:
    void setNomPortCom (string nomPortCOM)
    {
        this->nomPortCOM = nomPortCOM;
    }
    void setVitBau (int vitBau)
    {
        this->vitBau=vitBau;
    }
    void setTaiDon(int taiDon)
    {
        this->taiDon=taiDon;
    }
    void setNbrBitSto(int nbrBitSto)
    {
        this->nbrBitSto=nbrBitSto;
    }
    string getNomPortCOM()
    {
        return nomPortCOM;
    }
    int getVitBau()
    {
        return vitBau;
    }
    int getTaiDon()
    {
        return taiDon;
    }
    int getNbrBitSto()
    {
        return nbrBitSto;
    }

    ParametreRS232(string nomPortCOM, int vitBau, int taiDon, int nbrBitSto)
    {
        setNomPortCom(nomPortCOM);
        setVitBau(vitBau);
        setTaiDon(taiDon);
        setNbrBitSto(nbrBitSto);
    }

    void afficherParametre()
    {
        cout << "Le port serie est : " << nomPortCOM << endl;
        cout << "La vitesse est de : " << vitBau << " bauds.\n";
        cout << "taiDon = " << taiDon << endl;
        cout << "nbrBitSto = " << nbrBitSto << endl;
    }

    void chargerParametresRS232(string nomFichier)
    {
        FichierConfiguration fg("parametresRS232.txt");  ///On vient demander le nom du fichier à recupérer.
        if (fg.getOk())
        {
            this->nomPortCOM = fg.lireValeur("nomPortCOM");  ///On met la valeur mise dans le fichier dans this->nomPortCOM
            this->vitBau = stoi(fg.lireValeur("vitBau"));   ///On met la valeur mise dans le fichier dans this->vitBau
            this->taiDon = stoi(fg.lireValeur("donnees"));  ///On met la valeur mise dans le fichier dans this->taiDon
            this->nbrBitSto = stoi(fg.lireValeur("stop"));   ///On met la valeur mise dans le fichier dans this->nbrBitSto
            ///Stoi convertit les strings en entier.

        }
    }
};
class RS232
{
protected:
    bool etatPort;
    serialib serial;
    ParametresRS232 paramRS232;
public:
    bool ouvrirPort()   ///Fonction ouverture du port
    {
        paramRS232.chargerParametresRS232("parametresRS232");
        if(serial.openDevice(paramRS232.getNomPortCOM().c_str(), paramRS232.getVitBau())==1)   /// Si le port série est connecté COM"NB"
            ///avec une vitesse de : n
        {
            printf("\nOuverture du port : %s\n", paramRS232.getNomPortCOM().c_str());     ///Affiche un message de connexion
            etatPort = true;                                  ///Renvoie vrai au bool etatPort
            return true;  ///Retourne vrai
        }
        else
        {
            printf("\nErreur d'ouverture du port : %s\n", paramRS232.getNomPortCOM().c_str());
            etatPort = false;  ///Sinon renvoie faux au bool etatPort
            return false;   ///Retourne faux
        }

    }
    bool fermerPort()         ///Fonction fermeture du port
    {
        if(etatPort==true)
        {
            serial.closeDevice();
            printf("\nFermeture du port : %s\n\n", paramRS232.getNomPortCOM().c_str());
            etatPort = false;
            return true;
        }
        else
            return false;
    }
    int lireChar(char * buf, int timeout)
    {
        return serial.readChar(buf, timeout);   /// On vient directement prendre dans la librairie
                                                ///Serial la fonction readChar();
    }
    int lireTrame (unsigned char *buf, int taille , int timeout)
    {
        return serial.readBytes(buf, taille, timeout);  /// On vient directement prendre dans la librairie
                                                        ///Serial la fonction readBytes();
    }
    /*int lireOctet(char *buffer)
    {
        char c;
        if(etatPort == true)
        {
            do
            {
                serial.readChar(&c,10000);
                //cout << c;
            }
            while (c!='\0');
        }
    }*/
    /*int lireTrame(unsigned char *trame, char carDeb, char carFin)
    {
        char c;
        int index = 0;
        if(etatPort == true)
        {
            cout << "\n-----Lire TRAME-----\n";

            do
            {
                serial.readChar(&c,10000);
            }
            while (c!=carDeb);

            do
            {
                if(serial.readChar(&c)!=0)
                {
                    trame[index++] = c;
                    trame[index]='\0';
                }
            }
            while (c!=carFin);
            //cout << trame << endl;

        }
    }*/

    int ecrireChar(char byte)
    {
        if(etatPort == true)
        {
            cout << "\n-----Ecrire OCTET-----\n";
            serial.writeChar(byte);
            cout << byte;
        }

    }
    int ecrireTrame(unsigned char *trame, int taille)
    {
        if(etatPort == true)
        {
            serial.writeBytes(trame, taille);
        }
    }

    void setEtatPort(bool etatPort)
    {
        this -> etatPort = etatPort;
    }
    bool getEtatPort()
    {
        return this->etatPort;
    }
    /*void setTimeout(unsigned int timeOut)
    {
        this->serial.setTimeOut(timeOut);
    }*/

};
/*class SecteurAngle
{
protected:
    float angMin;
    float angMax;
public:
    SecteurAngle(float angMin, float angMax)
    {

    }
};*/
class DonneesLidar
{
protected :
    float secteurAngulaire; /// angle chosit est de 360°
    float pasAngle;    ///  pasAngle = 1°
    float *tabDistances;      /// Allocation dynamique
    int nbrAng;
public :
    DonneesLidar()
    {
    }
    DonneesLidar(float secteurAngulaire, float pasAngle)
    {
        this->secteurAngulaire = secteurAngulaire;
        this->pasAngle = pasAngle;
        this->nbrAng = this->secteurAngulaire / this->pasAngle;
        this->tabDistances = new float[nbrAng];  /// Allouer un tableau dynamique adapté au LIDAR

        cout << "Const donnneesLidar " << nbrAng << " " <<  this->tabDistances << endl;
    }
    ~DonneesLidar()   /// Destruction du constructeur DonneesLidar
    {
        delete this->tabDistances;  /// Supprimer le tableau dynamique
    }
    int getNbrAng()
    {
        return this->nbrAng;
    }
    float *getTabDistances()
    {
        return this->tabDistances;
    }
    void setTabDistances(float *tabDistances, int nbrAng)  /// Pour la classe LIDAR
    {
        /// lock_mutex
        for (int i=0; i<nbrAng; i++)
            this->tabDistances[i] = tabDistances[i];
        /// unlock_mutex
    }
    DonneesLidar getDonneesLidar()  /// Pour l'application
    {
        DonneesLidar tmp(this->secteurAngulaire, this->pasAngle);
        tmp.secteurAngulaire = this->secteurAngulaire;
        tmp.pasAngle = this->pasAngle;
        /// lock_mutex
        for (int i=0; i<this->nbrAng; i++)
            tmp.tabDistances[i] = this->tabDistances[i];
        /// unlock_mutex
        return tmp;
    }
    void afficher()
    {
        cout << "Donnees LIDAR : \n";
        for (int i=0; i<nbrAng; i++)
            cout << "dist[" << i << "] = " << this->tabDistances[i] << '\n';
    }
};
class Lidar
{
protected:
    bool finAcq;
    DonneesLidar donneesLidar;
public :
    RS232 rs232;
    bool connecter()
    {
        return rs232.ouvrirPort();
    }
    bool deconnecter()
    {
        return  rs232.fermerPort();
    }
public:
    Lidar()
    {

    }
    /*DonneesLidar getDonneesLidar(SecteurAngle secteurAngle, float pasAng)
    {
        ///this -> donneesLidar.secteurAngle = secteurAngle;
    }*/
    void demarrerRotation()
    {
        unsigned char trameDemRotatLidar[]= {0xA5,0x2C,0xE1,0xAA,0xBB,0xCC,0xDD};  ///Trame démarrage de la rotation "0x2C"
        rs232.ecrireTrame(trameDemRotatLidar,15);  ///Ecrire la trame
        cout << "La trame de mise en rotation vient d'etre envoyee.... \n\n" ; ///Message de prévention
    }
    void demarrerAcquisition()
    {
        unsigned char trameDemAcq[]= {0xA5,0x20,0xE1,0xAA,0xBB,0xCC,0xDD}; ///Trame démarrage d'acquisition "0x20"
        rs232.ecrireTrame(trameDemAcq,15);  ///Ecrire la trame
        cout << "La trame d'acquision vient d'etre envoyee.... \n\n" ;  ///Message de prévention
    }
    void stopperAcquisition()
    {
        unsigned char trameStopAcq[]= {0xA5,0x21,0xE1,0xAA,0xBB,0xCC,0xDD}; ///Trame de stop acquisition "0x21"
        rs232.ecrireTrame(trameStopAcq,15); /// Ecrire trame
        cout << "Fin de l'acquisition." << endl; /// Message de prévention
    }
    bool stopperRotation()
    {
        unsigned char trameStop[]= {0xA5,0x25,0xE1,0xAA,0xBB,0xCC,0xDD};  ///Trame de stop Rotation "0x25"
        rs232.ecrireTrame(trameStop,15);  ///Ecrire Trame
        cout << "Fin de la rotation." << endl;   ///Message de fin
    }
    /*int lireTrameLidar()
    {

        unsigned char buf;
        unsigned char header[6];
        unsigned char NuageP[1800];
        float tabDistances[360]; /// = NULL;
        //tabDistances = donneesLidar.getTabDistances();
        //cout << "TabDistances = " << tabDistances << endl;
        while(1)
        {
            int c= rs232.serial.readChar((char *)&buf, 1000);
            if (buf==0xA5)
            {
                int angle, dist;
                c = rs232.serial.readBytes(header,6);
                c = rs232.serial.readBytes(NuageP, 1800);
                for (int i = 0; i<360; i++)
                {
                    angle = ((NuageP[1+5*i]+256*NuageP[2+5*i])/10)-1;
                    if (angle<0)
                        angle = 0;
                    if(angle>359)
                        angle=0;
                    dist = NuageP[3+5*i]+256*NuageP[4+5*i];
                    if(angle==359)
                        cout << "distance =  " << dist << "\n";
                    tabDistances[angle]=dist;
                }
            }
        }
    }*/
};
class Thread
{
protected :
    bool finThread;
    thread *ptrThread;
    string nomThread;
    bool etatThread;

public :
    Thread()
    {
        this->nomThread = "Thread anonyme";
        this->ptrThread = NULL;
    }
    Thread(string nomThread)
    {
        this->nomThread = nomThread;
        this->ptrThread = NULL;
    }
    ~Thread()
    {
        delete this->ptrThread;
        this->ptrThread = NULL;
    }
    bool getEtatThread()
    {
        return etatThread;
    }
    void demarrer()
    {
        if ( this->ptrThread!=NULL)
        {
            cout << "Demarrage du thread '" << this->nomThread << "' non valide : le thread est deja en cours\n";
            return ;
        }
        this->finThread = false;
        ptrThread = new thread(&Thread::run, this);
    }
    void arreter()
    {
        if ( this->ptrThread==NULL)
        {
            cout << "Arrête du thread '" << this->nomThread << "' non valide : pas de thread en cours\n";
            return ;
        }
        this->finThread = true;
        this->ptrThread->join();
        delete this->ptrThread;
        this->ptrThread = NULL;
    }
    virtual void run()
    {
        etatThread = true;     ///Signifie que le thread est lancé sans soucis
        while (!finThread)
        {
            cout << "fonction thread" << endl;
            Sleep(500);
        }
    };
    void attentreFinThread()
    {
        this->ptrThread->join();
    }
};
class ThreadLidar : public Thread
{
protected :
    Lidar lidar;
public :
    ThreadLidar(string nomThread) : Thread(nomThread)
    {
    }
    void run()
    {
        if (lidar.connecter()==true)
        {
            lidar.demarrerRotation();
            Sleep(100);
            lidar.demarrerAcquisition();
            Sleep(500);
            unsigned char buf;
            unsigned char trame[1806];   ///1806 est la longueur de la trame 6 bits d'entête et 1800 points de données
            float tabDistances[360]; /// = NULL;

            //tabDistances = donneesLidar.getTabDistances();
            //cout << "TabDistances = " << tabDistances << endl;

            while (!finThread)
            {

                int c= lidar.rs232.lireChar((char *)&buf, 1000);
                if (buf==0xA5)
                {
                    int angle, dist;
                    c = lidar.rs232.lireTrame(trame, 1806, 1000);  ///1806 est la longueur de la trame 6 bits d'entête
                                                                   ///et 1800 points de données

                    for (int i = 0; i<360; i++)
                    {
                        angle = ((trame[6+1+5*i]+256*trame[6+2+5*i])/10)-1;
                        if (angle<0 || angle>359)
                        {
                            angle = 0;
                        }
                        dist = trame[6+3+5*i]+256*trame[6+4+5*i];
                        if(angle==359)
                            cout << "distance =  " << dist << "\n";
                        tabDistances[angle]=dist;
                    }
                }
            }
            lidar.stopperAcquisition();
            Sleep(500);
            lidar.stopperRotation();
            lidar.deconnecter();
        }
        else
        {
            cout <<"Erreur de connexion au lidar\n";
        }
    }
};


int main()
{
    ThreadLidar thLidar("Thread Lucas");
    thLidar.demarrer();
    Sleep(15000);
    thLidar.arreter();
    exit(0);





















/*
        unsigned char recu[255];
        char envoyer;
        RS232 rs232;
        rs232.ouvrirPort();
        cout << endl;
        rs232.ecrireChar('L');
        cout << endl;
        rs232.ecrireTrame("$GP656S5D\n",12);
        cout << endl;
        rs232.fermerPort();*/





/*
    RS232 rs232;
    unsigned char *recu;
    char *recu2;
    char envoyer;
    int taille = 42;
    rs232.ecrireChar(envoyer);
    cout << endl;
    rs232.lireChar(recu2, 1000);
    cout << endl;
    rs232.ecrireTrame(recu,taille);
    cout << endl;
    rs232.lireTrame(recu, taille, 10000);
*/



/*




     ParametresRS232 parametresRS232;
      parametresRS232.chargerParametresRS232("parametresRS232.txt");*/

    /* Lidar lidar;
     if (lidar.connecter()==true)
     {
         lidar.demarrerRotation();
         Sleep(100);
         lidar.demarrerAcquisition();
         Sleep(3000);
         //lidar.lireTrameLidar();
         lidar.stopperAcquisition();
         Sleep(3000);
         lidar.stopperRotation();
         lidar.deconnecter();

     }
     else
     {
         cout <<"Erreur de connexion au lidar\n";
     }





        unsigned char recu[255];
        char envoyer;
        RS232 rs232;
        rs232.ouvrirPort();
        cout << endl;
        ///rs232.ecrireChar('L');
        cout << endl;
        rs232.ecrireTrame("$GP656S5D\n",12);
        cout << endl;
        rs232.fermerPort();
*/
    return 0;
}


