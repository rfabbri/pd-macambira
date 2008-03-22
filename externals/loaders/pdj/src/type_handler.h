#include <jni.h>
#include "m_pd.h"

#define DataTypes_ALL 15
#define DataTypes_ANYTHING 15
#define DataTypes_FLOAT 2
#define DataTypes_INT 1
#define DataTypes_LIST 4
#define DataTypes_MESSAGE 8

int jatom2atom(JNIEnv *env, jobject jatom, t_atom *atom);
int jatoms2atoms(JNIEnv *env, jobjectArray jatoms, int *nb_atoms, t_atom *atoms);

jobject atom2jatom(JNIEnv *env, t_atom *atom);
jobjectArray atoms2jatoms(JNIEnv *env, int argc, t_atom *argv);

t_symbol *jstring2symbol(JNIEnv *env, jstring strvalue);
t_pd *findPDObject(t_symbol *name);
