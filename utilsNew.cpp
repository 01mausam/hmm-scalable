/*
 
 Copyright (c) 2012-2014, Michael (Mikhail) Yudelson
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the Michael (Mikhail) Yudelson nor the
 names of other contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */

#include "utils.h"
using namespace std;

// project of others
int compareNumber (const void * a, const void * b) {
	NUMBER cmp = ( *(NUMBER*)a - *(NUMBER*)b );
	return -1*(cmp<0) + 0 + (cmp>0)*1;
}

int compareNumberRev (const void * a, const void * b) {
//	return ( *(NUMBER*)b - *(NUMBER*)a );
	NUMBER cmp = ( *(NUMBER*)b - *(NUMBER*)a );
	return -1*(cmp<0) + 0 + (cmp>0)*1;
}

void qsortNumber(NUMBER* ar, NPAR size) {
	qsort (ar, size, sizeof(NUMBER), compareNumber);
}

void qsortNumberRev(NUMBER* ar, NPAR size) {
	qsort (ar, size, sizeof(NUMBER), compareNumberRev);
}

// refer to http://arxiv.org/abs/1101.6081 for source
void projsimplex(NUMBER* y, NPAR size) {

	bool bget = false;
	NUMBER *s = init1D<NUMBER>(size);// init1<NUMBER>(size);
	cpy1D<NUMBER>(y, s, size);
	
	qsortNumberRev(s, size);
	NUMBER tmpsum = 0, tmax = 0;

	for(NPAR i=0; i<(size-1); i++) {
		tmpsum = tmpsum + s[i];
		tmax = (tmpsum - 1)/(i+1);
		if(tmax >= s[i+1]) {
			bget = true;
			break;
		}
	}
	if(!bget) tmax = (tmpsum + s[size-1] -1)/size;
	free(s);

	for(NPAR i=0; i<size; i++)
		y[i] = ((y[i]-tmax)<0)?0:(y[i]-tmax);
}


// project of my own
bool issimplex(NUMBER* ar, NPAR size) {
	NUMBER sum = 1;
	for(NPAR i=0; i<size; i++) {
		sum -= ar[i];
		if( ar[i] < 0 || ar[i] > 1)
			return false;
	}
	return fabs(sum)<SAFETY;
}

bool issimplexbounded(NUMBER* ar, NUMBER *lb, NUMBER *ub, NPAR size) {
	NUMBER sum = 1;
	for(NPAR i=0; i<size; i++) {
		sum -= ar[i];
		if( ar[i] < lb[i] || ar[i] > ub[i])
			return false;
	}
	return fabs(sum)<SAFETY;
}


void projectsimplex(NUMBER* ar, NPAR size) {
	NPAR i, num_at_hi, num_at_lo; // number of elements at lower,upper boundary
	NPAR *at_hi = Calloc(NPAR, size);
	NPAR *at_lo = Calloc(NPAR, size);
	NUMBER err, lambda;
	while( !issimplex(ar, size)) {
        lambda = 0;
		num_at_hi = 0;
		num_at_lo = 0;
		err = -1; // so that of the sum is over 1, the error is 1-sum
		// threshold
		for(i=0; i<size; i++) {
			at_lo[i] = (ar[i]<0)?1:0;
			ar[i] = (at_lo[i]==1)?0:ar[i];
			num_at_lo += at_lo[i];

			at_hi[i] = (ar[i]>1)?1:0;
			ar[i] = (at_hi[i]==1)?1:ar[i];
			num_at_hi += at_hi[i];
			
			err += ar[i];
		}
		if (size > (num_at_hi + num_at_lo) )
			lambda = err / (size - (num_at_hi + num_at_lo));
		for(i=0; i<size; i++)
			ar[i] -= (at_lo[i]==0 && at_hi[i]==0)?lambda:0;
		
	} // until satisfied
	free(at_hi);
	free(at_lo);
}

void projectsimplexbounded(NUMBER* ar, NUMBER *lb, NUMBER *ub, NPAR size) {
	NPAR i, num_at_hi, num_at_lo; // number of elements at lower,upper boundary
	NPAR *at_hi = Calloc(NPAR, size);
	NPAR *at_lo = Calloc(NPAR, size);
	NUMBER err, lambda;
	while( !issimplexbounded(ar, lb, ub, size)) {
        lambda = 0;
		num_at_hi = 0;
		num_at_lo = 0;
		err = -1;
		// threshold
		for(i=0; i<size; i++) {
			at_lo[i] = (ar[i]<lb[i])?1:0;
			ar[i] = (at_lo[i]==1)?lb[i]:ar[i];
			num_at_lo += at_lo[i];
			
			at_hi[i] = (ar[i]>ub[i])?1:0;
			ar[i] = (at_hi[i]==1)?ub[i]:ar[i];
			num_at_hi += at_hi[i];
			
			err += ar[i];
		}
		if (size > (num_at_hi + num_at_lo) )
			lambda = err / (size - (num_at_hi + num_at_lo));
		for(i=0; i<size; i++)
			ar[i] -= (at_lo[i]==0 && at_hi[i]==0)?lambda:0;
		
	} // until satisfied
	free(at_hi);
	free(at_lo);
}


//template<typename T> void toZero1D(T* ar, NDAT size) {
//	for(NDAT i=0; i<size; i++)
//		ar[i] = 0;    
//}
//
//template<typename T> void toZero2D(T** ar, NDAT size1, NDAT size2) {
//	for(NDAT i=0; i<size1; i++)
//        for(NDAT j=0; j<size2; j++)
//            ar[i][j] = 0;
//}
//
//template<typename T> void toZero3D(T*** ar, NDAT size1, NDAT size2, NDAT size3) {
//	for(NDAT i=0; i<size1; i++)
//        for(NDAT j=0; j<size2; j++)
//            for(NDAT l=0; l<size3; l++)
//                ar[i][j][l] = 0;
//}


//void toZero1<NUMBER>(NUMBER *ar, NDAT size) { // 1D array
//	NDAT i;
//	for(i=0; i<size; i++)
//		ar[i] = 0;	
//}
//
//void toZero2<NUMBER>(NUMBER **ar, NDAT size1, NPAR size2) { // 1D array
//	NDAT i;
//	NPAR j;
//	for(i=0; i<size1; i++)
//		for(j=0; j<size2; j++)
//			ar[i][j] = 0;
//}
//
//void toZero3<NUMBER>(NUMBER ***ar, NDAT size1, NPAR size2, NPAR size3) { // 1D array
//	NDAT i;
//	NPAR j, m;
//	for(i=0; i<size1; i++)
//		for(j=0; j<size2; j++)
//            for(m=0; m<size2; m++)
//                ar[i][j][m] = 0;
//}

//template<typename T> T* init1D(NDAT size) {
//    T* ar = Calloc(T, size);
//    return ar; 
//}
//
//template<typename T> T** init2D(NDAT size1, NDAT size2) {
//	T** ar = (T **)Calloc(T *,size1);
//	for(NDAT i=0; i<size1; i++)
//		ar[i] = (T *)Calloc(T, size2);
//	return ar;
//}
//
//template<typename T> T*** init3D(NDAT size1, NDAT size2, NDAT size3) {
//	NDAT i,j;
//	T*** ar = Calloc(T **, size1);
//	for(i=0; i<size1; i++) {
//		ar[i] = Calloc(T*, size2);
//		for(j=0; j<size2; j++)
//			ar[i][j] = Calloc(T, size3);
//	}
//	return ar;
//}

//NUMBER* init1<NUMBER>(NDAT size) {
//	NUMBER* ar = Calloc(NUMBER,size);
////	toZero1<NUMBER>(ar, size);
//	return ar;
//}

//NUMBER** init2<NUMBER>(NDAT size1, NPAR size2) {
//	NDAT i;
//	NUMBER** ar = (NUMBER **)Calloc(NUMBER *,size1);
//	for(i=0; i<size1; i++)
//		ar[i] = (NUMBER *)Calloc(NUMBER, size2);
////	toZero2<NUMBER>(ar, size1, size2);
//	return ar;
//}

//NUMBER*** init3<NUMBER>(NCAT size1, NDAT size2, NPAR size3) {
//	NCAT i;
//	NDAT j;
//	NUMBER*** ar = Calloc(NUMBER **, size1);
//	for(i=0; i<size1; i++) {
//		ar[i] = Calloc(NUMBER*, size2);
//		for(j=0; j<size2; j++)
//			ar[i][j] = Calloc(NUMBER, size3);
//	}
//	return ar;
//}
//
//NPAR** init2DNCat(NCAT size1, NCAT size2) {
//	NCAT i;
//	NPAR** ar = Calloc(NPAR *,size1);
//	for(i=0; i<size1; i++)
//		ar[i] = Calloc(NPAR, size2);
//	return ar;
//}

//template<typename T> void free2D(T** ar, NDAT size1) {
//	for(NDAT i=0; i<size1; i++)
//		free(ar[i]);
//	free(ar);
////    &ar = NULL;
//}
//
//template<typename T> void free3D(T*** ar, NDAT size1, NDAT size2) {
//	for(NDAT i=0; i<size1; i++) {
//		for(NDAT j=0; j<size2; j++)
//			free(ar[i][j]);
//		free(ar[i]);
//	}
//	free(ar);
////    &ar = NULL;
//}

//void free2<NUMBER>(NUMBER **ar, NDAT size1) {
//	for(NDAT i=0; i<size1; i++)
//		free(ar[i]);
//	free(ar);
//}

//void free3<NUMBER>(NUMBER ***ar, NCAT size1, NDAT size2) {
//	for(NCAT i=0; i<size1; i++) {
//		for(NDAT j=0; j<size2; j++)
//			free(ar[i][j]);
//		free(ar[i]);
//	}
//	free(ar);
//}

//void free2DNCat(NCAT **ar, NCAT size1) {
//	NCAT i;
//	for(i=0; i<size1; i++)
//		free(ar[i]);
//	free(ar);
//}

//template<typename T> void cpy1D(T* source, T* target, NDAT size) {
//    memcpy( target, source, sizeof(T)*size );
//}

//template<typename T> void cpy2D(T** source, T** target, NDAT size1, NDAT size2) {
//	for(NDAT i=0; i<size1; i++)
//		memcpy( target[i], source[i], sizeof(T)*size2 );
//}

//template<typename T> void cpy3D(T*** source, T*** target, NDAT size1, NDAT size2, NDAT size3) {
//	for(NDAT t=0; t<size1; t++)
//        for(NDAT i=0; i<size2; i++)
//            memcpy( target[t][i], source[t][i], sizeof(T)*size3 );
//}

//void cpy1<NUMBER>(NUMBER* source, NUMBER* target, NDAT size) {
//	memcpy( target, source, sizeof(NUMBER)*size );
//}

//void cpy2<NUMBER>(NUMBER** source, NUMBER** target, NDAT size1, NPAR size2) {
//	for(NDAT i=0; i<size1; i++)
//		memcpy( target[i], source[i], sizeof(NUMBER)*size2 );
//}

//void cpy3<NUMBER>(NUMBER*** source, NUMBER*** target, NDAT size1, NPAR size2, NPAR size3) {
//	for(NDAT t=0; t<size1; t++)
//        for(NPAR i=0; i<size2; i++)
//            memcpy( target[t][i], source[t][i], sizeof(NUMBER)*size3 );
//}

//template<typename T> void swap1D(T* source, T* target, NDAT size) {
//    T* buffer = init1D<T>(size); // init1<NUMBER>(size);
//	memcpy( target, buffer, sizeof(T)*size );
//	memcpy( source, target, sizeof(T)*size );
//	memcpy( buffer, source, sizeof(T)*size );
//    free(buffer);
//}
//
//template<typename T> void swap2D(T** source, T** target, NDAT size1, NDAT size2) {
//    T** buffer = init2D<T>(size1, size2);
//    cpy2D<T>(target, buffer, size1, size2);
//    cpy2D<T>(source, target, size1, size2);
//    cpy2D<T>(buffer, source, size1, size2);
//    free2D<T>(buffer, size1);
//}
//
//template<typename T> void swap3D(T*** source, T*** target, NDAT size1, NDAT size2, NDAT size3) {
//    T*** buffer = init3D<T>(size1, size2, size3);
//    cpy3D<T>(target, buffer, size1, size2, size3);
//    cpy3D<T>(source, target, size1, size2, size3);
//    cpy3D<T>(buffer, source, size1, size2, size3);
//    free3D<T>(buffer, size1, size2);    
//}

//void swap1<NUMBER>(NUMBER* source, NUMBER* target, NDAT size) {
//    NUMBER* buffer = init1D<NUMBER>(size); // init1<NUMBER>(size);
//	memcpy( target, buffer, sizeof(NUMBER)*size );
//	memcpy( source, target, sizeof(NUMBER)*size );
//	memcpy( buffer, source, sizeof(NUMBER)*size );
//    free(buffer);
//}

//void swap2<NUMBER>(NUMBER** source, NUMBER** target, NDAT size1, NPAR size2) {
//    NUMBER** buffer = init2<NUMBER>(size1, size2);
//    cpy2<NUMBER>(target, buffer, size1, size2);
//    cpy2<NUMBER>(source, target, size1, size2);
//    cpy2<NUMBER>(buffer, source, size1, size2);
//    free2<NUMBER>(buffer, size1);
//}

//void swap3<NUMBER>(NUMBER*** source, NUMBER*** target, NDAT size1, NPAR size2, NPAR size3) {
//    NUMBER*** buffer = init3<NUMBER>(size1, size2, size3);
//    cpy3<NUMBER>(target, buffer, size1, size2, size3);
//    cpy3<NUMBER>(source, target, size1, size2, size3);
//    cpy3<NUMBER>(buffer, source, size1, size2, size3);
//    free3<NUMBER>(buffer, size1, size2);
//}

NUMBER safe01num(NUMBER val) {
    //    val = (val<0)?0:((val>1)?1:val); // squeeze into [0,1]
    //	return val + SAFETY*(val==0) - SAFETY*(val==1); // then futher in
    return (val<=0)? SAFETY : ( (val>=1)? (1-SAFETY) : val );
}

NUMBER safe0num(NUMBER val) {
//    return (fabs(val)<SAFETY)?(SAFETY*(val>=0) + SAFETY*(val<0)*(-1)):val;
    return (val<SAFETY)?SAFETY:val;
}

NUMBER itself(NUMBER val) {
	return val;
}

NUMBER deprecated_fsafelog(NUMBER val) {
	return safelog(val + (val<=0)*SAFETY);
}

NUMBER safelog(NUMBER val) {
	return log(val + (val<=0)*SAFETY);
}

NUMBER sigmoid(NUMBER val) {
    return 1 / (1 + exp(-val));
}

NUMBER logit(NUMBER val) {
    NUMBER prob = (val<=0)? SAFETY : ( (val>=1)? (1-SAFETY) : val );
    return log( prob / (1-prob) );
//    return fsafelog(val/safe0num(1-val));
}

//#define logit(y)
//#define logit(y) 
//NUMBER logit(NUMBER val) {
//    //	return fsafelog( val / safe0num(1-val) );
//    return fastsafelog((val+(val==0)*SAFETY)/(1-val+(val>=1)*SAFETY));
//}

NUMBER sgn(NUMBER val) {
	return (0 < val) - (val < 0);
}

void add1DNumbersWeighted(NUMBER* sourse, NUMBER* target, NPAR size, NUMBER weight) {
	for(NPAR i=0; i<size; i++)
		target[i] = target[i] + sourse[i]*weight;
}

void add2DNumbersWeighted(NUMBER** sourse, NUMBER** target, NPAR size1, NPAR size2, NUMBER weight) {
	for(NPAR i=0; i<size1; i++)
		for(NPAR j=0; j<size2; j++)
			target[i][j] = target[i][j] + sourse[i][j]*weight;
}

bool isPasses(NUMBER* ar, NPAR size) {
	NUMBER sum = 0;
	for(NPAR i=0; i<size; i++) {
		if( ar[i]<0 || ar[i]>1)
			return false;
		sum += ar[i];
	}
	return sum==1;
}

bool isPassesLim(NUMBER* ar, NPAR size, NUMBER *lb, NUMBER* ub) {
	NUMBER sum = 0;
	for(NPAR i=0; i<size; i++) {
		if( (lb[i]-ar[i])>SAFETY || (ar[i]-ub[i])>SAFETY )
			return false;
		sum += ar[i];
	}
	return fabs(sum-1)<SAFETY;
}

// scale by smallest factor of 10 (max scaling by default
NUMBER doLog10Scale1D(NUMBER *ar, NPAR size) {
	NPAR i;
	NUMBER min_10_scale = 1000, max_10_scale = 0, candidate;
	for(i=0; i<size; i++) {
		if( fabs(ar[i]) < SAFETY ) // 0 gradient
			continue;
		candidate = floor( log10( fabs(ar[i]) ) );
		if(candidate < min_10_scale)
			min_10_scale = candidate;
		candidate = ceil( log10( fabs(ar[i]) ) );
		if(candidate > max_10_scale)
			max_10_scale = candidate;
	}
	min_10_scale++;
	max_10_scale++;
	if(max_10_scale > 0)
		for(i=0; i<size; i++)
			ar[i] = ar[i] / pow(10, max_10_scale);
//	if(min_10_scale<1000)
//		for(i=0; i<size; i++)
//			ar[i] = ar[i] / pow(10, min_10_scale);
	return pow(10, max_10_scale);
}

// scale by smallest factor of 10  (max scaling by default
NUMBER doLog10Scale2D(NUMBER **ar, NPAR size1, NPAR size2) {
	NPAR i,j;
	NUMBER min_10_scale = 1000, max_10_scale = 0, candidate;
	for(i=0; i<size1; i++)
		for(j=0; j<size2; j++) {
			if( fabs(ar[i][j]) < SAFETY ) // 0 gradient
				continue;
			candidate = floor( log10( fabs(ar[i][j]) ) );
			if(candidate < min_10_scale)
				min_10_scale = candidate;
			candidate = ceil( log10( fabs(ar[i][j]) ) );
			if(candidate > max_10_scale)
				max_10_scale = candidate;
		}
	min_10_scale++;
	max_10_scale++;
	if(max_10_scale >0 )
		for(i=0; i<size1; i++)
			for(j=0; j<size2; j++)
				ar[i][j] = ar[i][j] / pow(10, max_10_scale);
//	if(min_10_scale<1000)
//		for(i=0; i<size1; i++)
//			for(j=0; j<size2; j++)
//				ar[i][j] = ar[i][j] / pow(10, min_10_scale);
	return pow(10, max_10_scale);
}


// Gentle - as per max distance to go toward extreme value of 0 or 1
NUMBER doLog10Scale1DGentle(NUMBER *grad, NUMBER *par, NPAR size) {
	NPAR i;
	NUMBER max_10_scale = 0, candidate, min_delta = 1, max_grad = 0;
	for(i=0; i<size; i++) {
		if( fabs(grad[i]) < SAFETY ) // 0 gradient
			continue;
        // scale
        if(max_grad < fabs(grad[i]))
            max_grad = fabs(grad[i]);
		candidate = ceil( log10( fabs(grad[i]) ) );
		if(candidate > max_10_scale)
			max_10_scale = candidate;
        // delta: if grad<0 - distance to 1, if grad>0 - distance to 0
        candidate = (grad[i]<0)*(1-par[i]) + (grad[i]>0)*(par[i]) +
            ( (fabs(par[i])< SAFETY) || (fabs(1-par[i])< SAFETY) ); // these terms are there to avoid already extreme 0, 1 values;
        if( candidate < min_delta)
            min_delta = candidate;
	}
    max_grad = max_grad / pow(10, max_10_scale);
    if(max_10_scale > 0)
        for(i=0; i<size; i++)
        grad[i] = ( 0.95 * min_delta / max_grad) * grad[i] / pow(10, max_10_scale);
    return ( 0.95 * min_delta / max_grad ) / pow(10, max_10_scale);
}

// Gentle - as per max distance to go toward extreme value of 0 or 1
NUMBER doLog10Scale2DGentle(NUMBER **grad, NUMBER **par, NPAR size1, NPAR size2) {
	NPAR i,j;
	NUMBER max_10_scale = 0, candidate, min_delta = 1, max_grad = 0;
	for(i=0; i<size1; i++)
		for(j=0; j<size2; j++) {
			if( fabs(grad[i][j]) < SAFETY ) // 0 gradient
				continue;
            // scale
            if(max_grad < fabs(grad[i][j]))
                max_grad = fabs(grad[i][j]);
			candidate = ceil( log10( fabs(grad[i][j]) ) );
			if(candidate > max_10_scale)
				max_10_scale = candidate;
            // delta: if grad<0 - distance to 1, if grad>0 - distance to 0
            candidate = (grad[i][j]<0)*(1-par[i][j]) + (grad[i][j]>0)*(par[i][j]) +
                ( (fabs(par[i][j])< SAFETY) || (fabs(1-par[i][j])< SAFETY) ); // these terms are there to avoid already extreme 0, 1 values
            if( candidate < min_delta)
                min_delta = candidate;
		}
    max_grad = max_grad / pow(10, max_10_scale);
    if(max_10_scale >0 )
		for(i=0; i<size1; i++)
			for(j=0; j<size2; j++)
				grad[i][j] = ( 0.95 * min_delta / max_grad) * grad[i][j] / pow(10, max_10_scale);
	return ( 0.95 * min_delta / max_grad ) / pow(10, max_10_scale);
}



// for skill of group
void zeroTags(NCAT ix1, NCAT n, struct data* data) { // set counts in data sequences to zero
	for(NCAT q=0; q<n; q++)
		data[ix1+q].tag = 0;
}

void zeroCounts(NCAT ix1, NCAT n, struct data* data) {
	for(NCAT q=0; q<n; q++)
		data[ix1+q].cnt = 0;
}


// whether one value is no less than 20% of the sum
bool isBalancedArray(NUMBER *ar, NPAR size) {
	NPAR i;
	NUMBER sum = 0;
	for(i=0; i<size; i++)
		sum += ar[i];
	for(i=0; i<size; i++)
		if( ar[i]/sum < 0.2)
			return false;
	return true;
}


//// log-scaled math
//NUMBER eexp(NUMBER x) {
//	return (x>=LOGZERO)?0:exp(x);
//}
//
//NUMBER eln(NUMBER x) {
//	if(x==0 || (x>0 && x<SAFETY))
//		return LOGZERO;
//	else if(x>0) 
//		return safelog(x);
//	else {
//		printf("Error, log of negative value!\n");
//		return 1/SAFETY;
//	}
//}
//
//NUMBER elnsum(NUMBER eln_x, NUMBER eln_y) {
//	if( (eln_x>=LOGZERO) || (eln_y>=LOGZERO) ) {
//		if(eln_x>=LOGZERO)
//			return eln_y;
//		else
//			return eln_x;
//	} else {
//		if( eln_x >  eln_y)
//			return eln_x + eln(1 + exp(eln_y-eln_x));
//		else
//			return eln_x + eln(1 + exp(eln_x-eln_y));
//	}
//}
//
//NUMBER elnprod(NUMBER eln_x, NUMBER eln_y) {
//	if( (eln_x>=LOGZERO) || (eln_y>=LOGZERO) )
//		return LOGZERO;
//	else		
//		return eln_x + eln_y;
//}


//
// The heavy end - common functionality
//

void set_param_defaults(struct param *param) {
	// configurable - set
	param->tol                   = 0.01;
	param->time                  = 0;
	param->maxiter               = 200;
	param->quiet                 = 0;
	param->single_skill          = 0;
	param->structure			 = 1; // default is by skill
	param->solver				 = 2; // default is Gradient Descent
	param->solver_setting		 = -1; // -1 - not set
    param->metrics               = 0;
    param->metrics_target_obs    = 0;
    param->predictions           = 0;
    param->binaryinput           = 0;
	param->C                     = 0;
	param->init_params			 = Calloc(NUMBER, 5);
	param->init_params[0] = 0.5; // PI[0]
	param->init_params[1] = 1.0; // p(not forget)
	param->init_params[2] = 0.4; // p(learn)
	param->init_params[3] = 0.8; // p(not slip)
	param->init_params[4] = 0.2; // p(guess)
	param->param_lo				= Calloc(NUMBER, 10);
	param->param_lo[0] = 0; param->param_lo[1] = 0; param->param_lo[2] = 1; param->param_lo[3] = 0; param->param_lo[4] = 0; 
	param->param_lo[5] = 0; param->param_lo[6] = 0; param->param_lo[7] = 0; param->param_lo[8] = 0; param->param_lo[9] = 0; 
	param->param_hi				= Calloc(NUMBER, 10);
	param->param_hi[0] = 1.0; param->param_hi[1] = 1.0; param->param_hi[2] = 1.0; param->param_hi[3] = 0.0; param->param_hi[4] = 1.0; 
	param->param_hi[5] = 1.0; param->param_hi[6] = 1.0; param->param_hi[7] = 0.3; param->param_hi[8] = 0.3; param->param_hi[9] = 1.0; 
	param->cv_folds = 0;
	param->cv_strat = 'g'; // default group(student)-stratified
    param->cv_target_obs = 0; // 1st state to validate agains by default, cv_folds enables cross-validation
    param->multiskill = 0; // single skill per ovservation by default
    // vocabilaries
    param->map_group_fwd = NULL;
    param->map_group_bwd = NULL;
    param->map_step_fwd = NULL;
    param->map_step_bwd = NULL;
    param->map_skill_fwd = NULL;
    param->map_skill_bwd = NULL;
	// derived from data - set to 0
	param->N  = 0; //will be dynamically set in read_data_...()
	param->nS = 2;
	param->nO = 0;
	param->nG = 0;
	param->nK = 0;
	param->nI = 0;
	// data
    param->all_seq = NULL;
    param->nSeq = 0;
    param->nSeqNull = 0;
	param->k_nG = NULL;
	param->k_N = NULL;
	param->k_ix1stSeq = NULL;
//	param->k_numg = NULL;
//	param->k_data = NULL;
//	param->k_g_data = NULL;
//	param->g_numk = NULL;
//	param->g_data = NULL;
//	param->g_k_data = NULL;
    param->N_null = 0;
//    param->n_null_skill_group = 0;
//    param->null_skills = NULL;
	// fitting specific - Armijo rule, Wolfe criterions
	param->ArmijoC1            = 1e-4;				
	param->ArmijoC2            = 0.9;				
	param->ArmijoReduceFactor  = 2;//1/0.9;//
	param->ArmijoSeed          = 1; //1; - since we use smooth stepping 1 is the only thing we need
    param->ArmijoMinStep       = 0.001; //  0.000001~20steps, 0.001~10steps
    // coord descend
    param->first_iteration_qualify = 0;
    param->iterations_to_qualify   = 2;
    
}

void initDat(data *dat) {
    dat->n = 0;
    dat->id = -1;
    dat->tag = 0;
//    dat->ptr = 0;
    dat->cnt = 0;
    dat->ix1st = 0;
//    dat->ix = NULL;
//    dat->alpha = NULL;
//    dat->beta = NULL;
//    dat->gamma = NULL;
//    dat->xi = NULL;
//    dat->c = NULL;
//    dat->time = NULL;
    dat->p_O_param = 0.0;
//    dat->loglik = 0.0;
}

//void destroyDat(data *dat, bool has_time) {
//    if( dat->ix != NULL)
//        free(dat->ix);
//    if(has_time && dat->time != NULL )
//        free( dat->time );
//}

void destroy_input_data(struct param *param) {
	if(param->init_params != NULL) free(param->init_params);
	if(param->param_lo != NULL) free(param->param_lo);
	if(param->param_hi != NULL) free(param->param_hi);
	
    // data - checks if pointers to data are null anyway (whether we delete linear columns of data or not)
	if(param->dat_obs != NULL) delete param->dat_obs;
	if(param->dat_tix != NULL) free(param->dat_tix);
	if(param->dat_group != NULL) delete param->dat_group;
	if(param->dat_item != NULL) delete param->dat_item;
	if(param->dat_skill != NULL) delete param->dat_skill;
	if(param->dat_multiskill != NULL) delete param->dat_multiskill;
	if(param->dat_time != NULL) delete param->dat_time;
    
    if(param->dat_multiskill_seq != NULL) {
        for(NDAT t=0; t<param->N; t++)
            free(param->dat_multiskill_seq[t]);
        free(param->dat_multiskill_seq);
    }
    if(param->dat_skill_seq != NULL) free(param->dat_skill_seq); // this CAN be freed
    
    
//    // not null skills
//    for(NDAT q=0;q<(param->nSeq+param->nSeqNull); q++) // including Null skills
//        destroyDat( &param->all_seq[q], param->time==1 );

    if(param->all_seq != NULL) free(param->all_seq); // ndat of them
//    if(param->k_data != NULL)   free(param->k_data); // ndat of them (reordered by k)
//    if(param->g_data != NULL)   free(param->g_data); // ndat of them (reordered by g)
//    if(param->k_g_data != NULL) free(param->k_g_data); // nK of them
//    if(param->g_k_data != NULL) free(param->g_k_data); // nG of them

	if( param->k_nG != NULL ) free(param->k_nG);
	if( param->k_N != NULL ) free(param->k_N);
	if( param->k_ix1stSeq != NULL ) free(param->k_ix1stSeq);
    if( param->k_t != NULL ) {
        for(NCAT k=0; k<(param->nK+1); k++)
            free(param->k_t[k]);
        free(param->k_t);
    }
    
//	if(param->k_numg != NULL)   free(param->k_numg);
//	if(param->g_numk != NULL)   free(param->g_numk);
//    // null skills
//    for(NCAT g=0;g<param->n_null_skill_group; g++)
//        free(param->null_skills[g].ix); // was obs
//    if(param->null_skills != NULL) free(param->null_skills);
    // vocabularies
    delete param->map_group_fwd;
    delete param->map_group_bwd;
    delete param->map_step_fwd;
    delete param->map_step_bwd;
    delete param->map_skill_fwd;
    delete param->map_skill_bwd;
}


//
// read/write solver info to a file
//
void writeSolverInfo(FILE *fid, struct param *param) {
    // solver id
    if( param->solver_setting>0 ) {
        fprintf(fid,"SolverId\t%d.%d.%d\n",param->structure,param->solver,param->solver_setting);        
    } else {
        fprintf(fid,"SolverId\t%d.%d\n",param->structure,param->solver);
    }
	// nK
    fprintf(fid,"nK\t%d\n",param->nK);
    // nG
    fprintf(fid,"nG\t%d\n",param->nG);
    // nS
    fprintf(fid,"nS\t%d\n",param->nS);
    // nO
    fprintf(fid,"nO\t%d\n",param->nO);
}

void readSolverInfo(FILE *fid, struct param *param, NDAT *line_no) {
    string s;
    int c, i1, i2;
    // SolverId
    fscanf(fid,"SolverId\t%i.%i", &i1, &i2);
    param->structure = (NPAR) i1;
    param->solver    = (NPAR) i2;
    fscanf(fid,"SolverId\t%hhu.%hhu", &param->structure, &param->solver);
    c = fscanf(fid,".%hhu\n", &param->solver_setting);
    if( c<1 ) {
        fscanf(fid,"\n");
        param->solver_setting = -1;
    }
    (*line_no)++;
	// nK
    fscanf(fid,"nK\t%hi\n",&param->nK);
    (*line_no)++;
    // nG
    fscanf(fid,"nG\t%hi\n",&param->nG);
    (*line_no)++;
    // nS
    fscanf(fid,"nS\t%hhu\n",&param->nS);
    (*line_no)++;
    // nO
    fscanf(fid,"nO\t%hhu\n",&param->nO);
    (*line_no)++;
}

//
// Handling blocking labels
//
void zeroTags(struct param* param) { // set counts in data sequences to zero
    for(NDAT q=0;q<(param->nSeq+param->nSeqNull); q++) // including Null skills
        param->all_seq[q].tag = 0;
}

void zeroCounts(struct param* param) { // set counts in data sequences to zero
    for(NDAT q=0;q<(param->nSeq+param->nSeqNull); q++) // including Null skills
        param->all_seq[q].cnt = 0;
}

////
//// clear up all forward/backward/etc variables for a skill-slice
////
//void RecycleFitData(NCAT xndat, struct data** x_data, struct param *param) {
//	NCAT x;
//	NDAT t;
//	for(x=0; x<xndat; x++) {
////        if( x_data[x][0].cnt != 0)
////            continue;
//		if( x_data[x][0].alpha != NULL ) {
//			free2D<NUMBER>(x_data[x][0].alpha, x_data[x][0].n);  // only free data here
//			x_data[x][0].alpha = NULL;
//		}
//		if( x_data[x][0].c != NULL ) {
//			free(x_data[x][0].c);  // only free data here
//			x_data[x][0].c = NULL;
//		}
//		if( x_data[x][0].beta != NULL ) {
//			free2D<NUMBER>(x_data[x][0].beta,  x_data[x][0].n); // only free data here
//			x_data[x][0].beta = NULL;
//		}
//		if( x_data[x][0].gamma != NULL ) {
//			free2D<NUMBER>(x_data[x][0].gamma, x_data[x][0].n); // only free data here
//			x_data[x][0].gamma = NULL;
//		}
//		if( x_data[x][0].xi != NULL ) {
//			for(t=0;t<x_data[x][0].n; t++)
//				free2D<NUMBER>(x_data[x][0].xi[t],  param->nS); // only free data here
//			x_data[x][0].xi = NULL;
//		}
//	}
//}

//
// working with time
//

// limits are the borders of time bins, there are nlimits+1 bins total,  bins 0:nlimits
NPAR sec_to_linear_interval(int time, int *limits, NPAR nlimits){
    for(NPAR i=0; i<nlimits; i++)
        if( time < limits[i] )
            return i;
    return nlimits;
}

// limits are the borders of time bins, there are nlimits+1 bins total
int time_lim_20HDWM[5] = {20*60, 60*60, 24*60*60, 7*24*60*60, 30*24*60*60}; // 20min, hour, day, week, month

// 9 categories: <2m, <20m, <1h, same day, next day, same week, next week, <30d, >=30d
NPAR sec_to_9cat(int time1, int time2, int *limits, NPAR nlimits) {
    int diff = time2 - time1;
    
    if(diff <0 ) {
        fprintf(stderr,"ERROR! time 1 should be smaller than time 2\n");
        return 0;
    } else if ( diff < (2*60) ) { // 20min
        return 0;
    } else if ( diff < (20*60) ) { // 20min
        return 1;
    } else if (diff < (60*60) ) { // 1h
        return 2;
    } else if(diff < (14*24*60*60)) { // detect date structures
        time_t t1 = (time_t)time1;
        time_t t2 = (time_t)time2;
        struct tm * ttm1 = localtime (&t1);
        struct tm * ttm2 = localtime (&t2);
        if( diff < (24*60*60) && ttm1->tm_mday==ttm2->tm_mday ) { // same day
            return 3;
        } else if(    diff < (2*24*60*60) &&                      // next day
                   ( (ttm2->tm_wday == (ttm1->tm_wday+1)) ||
                     (ttm1->tm_wday==6 && ttm2->tm_wday==0)
                   )
                  ) {
            return 4;
        } else if( diff < (7*24*60*60) && (ttm1->tm_wday<ttm2->tm_wday) ) { // this week
            return 5;
        } else { // next week
            return 6;
        }
    } else if(diff < (30*24*60*60)) { // less than 30 days
        return 7;
    } else if(diff >= (30*24*60*60)) { // 30 days or more
        return 8;
    }
    return 0;
}


//// write time intervals to file
//void write_time_interval_data(param* param, const char *file_name) {
//    if(param->time != 1) {
//        fprintf(stderr,"ERROR! Time data has not been read.\n");
//        return;
//    }
//    std::map<NCAT,std::string>::iterator it_g;
//    std::map<NCAT,std::string>::iterator it_k;
//    string group, skill;
//    data *dt;
//    // open file
//    FILE *fid = fopen(file_name,"w");
//    fprintf(fid,"Group\tKC\ttime1\ttime2\ttimediff\ttime_lim_20HDWM\tOutcome\n");
//    // for all groups
//    for(NCAT k=0; k<param->nK; k++) {
//        // for all KCs
//        it_k = param->map_skill_bwd->find(k);
//        for(NCAT g=0; g<param->k_numg[k]; g++) {
//            it_k = param->map_group_bwd->find(g);
//            dt = param->k_g_data[k][g];
//            // for times from 2 to N
//            for(NDAT t=1; t<dt->n; t++) {
////                NPAR code = sec_to_linear_interval(dt->time[t]-dt->time[t-1], time_lim_20HDWM, sizeof(time_lim_20HDWM)/sizeof(int));
//                NPAR code = sec_to_9cat(dt->time[t-1], dt->time[t], time_lim_20HDWM, sizeof(time_lim_20HDWM)/sizeof(int));
//                fprintf(fid,"%s\t%s\t%d\t%d\t%d\t%d\t%d\n", it_g->second.c_str(), it_k->second.c_str(), dt->time[t-1], dt->time[t], (dt->time[t]-dt->time[t-1]), code, 1-param->dat_obs->get( dt->ix[t] ) );
//            }// for times from 2 to N
//        }// for all KCs
//    }// for all groups
//    // close file
//    fclose(fid);
//}

// penalties
NUMBER L2penalty(param* param, NUMBER w) {
    NUMBER penalty_offset = 0.5;
    return (param->C > 0)? 0.5*param->C*fabs((w-penalty_offset)) : 0;
}

