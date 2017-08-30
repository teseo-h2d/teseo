/* Teseo-2 Plug-in
 * Copyright (C) 2005  Stefano Pintore, Matteo Quintiliani (the "Authors").
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE Authors BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the Authors of the
 * Software shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Authors.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "teseo_resample.h"
#include "teseo_path.h"
#include "teseo_env.h"
#include "teseo_bezier_point.h"
#include "teseo_gimp_extends.h"

// #include "teseo_path.h"
// #include "values.h"



void teseo_subsampling_strokes(double *strokes, glong *pn_strokes, int passo) {
    glong n_strokes = *pn_strokes;
    glong i;
    // printf("\nBEFORE n_strokes = %d\n", n_strokes);

    for(i=0; (i*passo) < n_strokes; i++) {
	strokes[(i*2)   ] = strokes[(i*2*passo)];
	strokes[(i*2) +1] = strokes[(i*2*passo) +1];
    }
    *pn_strokes = n_strokes / passo;

    // printf("\nAFTER pn_strokes = %d,   n_strokes = %d,   i * step = %d,\n", *pn_strokes, n_strokes, i*passo);
}


void teseo_progressive_resampling_strokes(double *strokes, glong *pn_strokes) {
    glong n_strokes = *pn_strokes;
    glong i, j;
    // printf("\nBEFORE campionascendentex_strokes n_strokes = %d\n", n_strokes);

    i = 1;	
    while(i < n_strokes) {
	if(strokes[((i-1)*2)] >= strokes[(i*2)]) {
	    for(j=i+1; j<n_strokes; j++) {
		strokes[((j-1)*2)   ] = strokes[(j*2)];
		strokes[((j-1)*2) +1] = strokes[(j*2)+1];
	    }
	    n_strokes--;
	} else {
	    i++;
	}
    }
    *pn_strokes = n_strokes;

    // printf("\nAFTER campionascendentex_strokes n_strokes = %d\n", n_strokes);
}


void teseo_resampling_bezier(gint32 g_image, gboolean sw_abscissa_ascendent, gdouble points_per_pixel)
{
    FILE *ftmp;
    FILE *fbezier;
    char pathname [255] ;
    gchar newpathname [255] ;
    gint32 path_closed, num_elementi_bezier, n;  
    int bezier_n_strokes;
    double *bezier_strokes = NULL;
    int i, kk;
    int max_n_strokes = 1024;
    glong n_strokes;
    gdouble *strokes = NULL;
    gdouble *punti_bezier = NULL;
    double Px[4], Py[4];

    // class Bezier bezier = Bezier(3);
    struct teseo_bezier_point tbp;
    teseo_bezier_point_init(&tbp, 3, NULL, NULL);

    const int sw_cast_int = 0;
    // char filetmp[] = "/tmp/last.bezier.strokes.txt";
    gchar filetmp[255];
    // char filebezier[] = "/tmp/last.bezier.txt";
    gchar filebezier[255];

    //mtheo sprintf(filetmp, "%s/tmp/last.bezier.strokes.txt", getenv_teseo(TESEO_DATA));
    // TODO
    g_sprintf(filetmp, "%s/last.bezier.strokes.txt", teseo_get_environment_path(TMPPATH) );
    //mtheo g_sprintf(filebezier, "%s/tmp/last.bezier.txt", getenv_teseo(TESEO_DATA));
    // TODO
    g_sprintf(filebezier, "%s/last.bezier.txt", teseo_get_environment_path(TMPPATH) );

    //richiedo il path corrente
    gint num_paths=0;
    //g_message("tipo interpolazione %d",pivals.inter_type);

    /*Se esiste una traccia esegue*/
    gimp_path_list (g_image, &num_paths);

    if (num_paths>0) {
	//prendo il nome del path corrente
	strcpy(pathname, gimp_path_get_current(g_image));

	teseo_gimp_path_get_points (g_image, pathname, &path_closed, &num_elementi_bezier, &punti_bezier);
	fbezier = fopen(filebezier, "wt");
	if(!fbezier) {
	    g_message("File \"%s\" not found!", filebezier);
	    exit(1);
	}
	for(kk=0; kk<num_elementi_bezier; kk++) {
	    fprintf(fbezier, "%f\n", punti_bezier[kk]);
	}
	fclose(fbezier);


	gimp_progress_init("Teseo - Resampling path . . .");
	gimp_progress_update(0.0);

	n_strokes = 0;
	strokes = (double *) g_malloc(sizeof(double) * ((max_n_strokes+2) * 2) );
	if(!strokes) {
	    g_message("Not enough free memory for strokes in on_btnBezier_clicked!");
	    exit(1);
	}		

	ftmp = fopen(filetmp, "wt");
	if(!ftmp) {
	    g_message("File \"%s\" not found!", filetmp);
	    exit(1);
	}

	n = 0;
	while((((n+3)*3) +2) < num_elementi_bezier) {
	    // punti_bezier[n*3   ] � la x
	    // punti_bezier[n*3 +1] � la y
	    // punti_bezier[n*3 +2] � il tipo 1 per ANCHOR 2 per CONTROL
	    // la sequenza dovrebbe essere 1221
	    // g_printf("%d (%f, %f) n_strokes %ld\n", n, punti_bezier[(n  )*3], punti_bezier[((n  )*3) +1], n_strokes);
	    if(			((punti_bezier[ (n   *3) +2]) == 1.0)
	    &&	((punti_bezier[((n+1)*3) +2]) == 2.0)
	    &&	((punti_bezier[((n+2)*3) +2]) == 2.0)
	    &&	((punti_bezier[((n+3)*3) +2]) == 1.0) ) {
		// imposto Px e Py con i valori contenuti in punti_bezier
		Px[0] = punti_bezier[(n  )*3];	Py[0] = punti_bezier[((n  )*3) +1];
		Px[1] = punti_bezier[(n+1)*3];	Py[1] = punti_bezier[((n+1)*3) +1];
		Px[2] = punti_bezier[(n+2)*3];	Py[2] = punti_bezier[((n+2)*3) +1];
		Px[3] = punti_bezier[(n+3)*3];	Py[3] = punti_bezier[((n+3)*3) +1];

		// bezier.setPoints(Px, Py);
		teseo_bezier_point_setPoints(&tbp, Px, Py);

		// mi faccio tornare i punti appartenenti alla curva con passo lungo le x uguale a 1
		// dopo dovr� ricampionare strokes con passo pivals.passo_bezier

		// bezier_n_strokes = bezier.getStrokes(1, &bezier_strokes, sw_cast_int);
		bezier_n_strokes = teseo_bezier_point_getStrokes(&tbp, points_per_pixel, &bezier_strokes, (n_strokes > 0)? strokes[(n_strokes-1)*2] : Px[0] - points_per_pixel, sw_abscissa_ascendent);

		// g_printf("(%f,%f) %d points from teseo_bezier_point_getStrokes()\n", Px[0], Py[0], bezier_n_strokes);

		// devo aggiungere bezier_strokes a strokes
		if((n_strokes + bezier_n_strokes) >= max_n_strokes) {
		    while(max_n_strokes < (n_strokes + bezier_n_strokes) ) {
			max_n_strokes += 2048;
		    }

		    strokes = (double *) g_realloc((void *) strokes, (sizeof(double) * ((max_n_strokes+2) * 2)));
		    if(!strokes) {
			g_message("teseo_resampling_bezier(): not enough free memory for strokes!");
			exit(1);
		    }
		}
		// g_printf("bezier_n_strokes %d; \n", bezier_n_strokes);
		for(i=0; i<bezier_n_strokes; i++) {
		    strokes[n_strokes*2   ]   = bezier_strokes[i*2];
		    strokes[(n_strokes*2) +1] = bezier_strokes[(i*2) +1];
		    n_strokes++;
		    if(sw_cast_int) {
			fprintf(ftmp, "%d %d\n", (int) bezier_strokes[i*2], (int) bezier_strokes[(i*2) +1]);
		    } else {
			fprintf(ftmp, "%f %f\n", bezier_strokes[i*2], bezier_strokes[(i*2) +1]);
		    }
		    if(n_strokes>=2) {
			/* if(strokes[(n_strokes-1)*2] == strokes[(n_strokes-2)*2]) { */
			if(fabs(strokes[(n_strokes-1)*2] - strokes[(n_strokes-2)*2]) < (points_per_pixel/2.0) ) {
			// g_printf("Resolution Bug Leo: Duplicazione coordinata in X = %d (Y = %d). Action: Scartata.\n", (int) bezier_strokes[i*2], (int) bezier_strokes[i*2 +1]);
			g_message("Coordinates duplicated in X = %f (Y = %f). Action: discarded.\n", bezier_strokes[i*2], bezier_strokes[i*2 +1]);
			n_strokes--;
			}
		    }
		}           	
		g_free(bezier_strokes);
		gimp_progress_update((gdouble) (num_elementi_bezier) / (gdouble) (n*3));
	    }

	    // con questo incremento di n ci sar� qualche ciclo a vuoto,
	    // ma facendo cos� mi sento pi� tranquillo
	    n++;
	}
	fclose(ftmp);

	gimp_progress_init("Teseo - Resampling path finished.");
	gimp_progress_update(0.0);

	// g_message("Fine del calcolo di Bezier.");

	/*
	if(sw_abscissa_ascendent) {
	    teseo_progressive_resampling_strokes(strokes, &n_strokes);
	}		
	*/

	/*
	if(passo_bezier > 1) {
	    // teseo_subsampling_strokes(strokes, &n_strokes, pivals.passo_bezier);
	    teseo_subsampling_strokes(strokes, &n_strokes, passo_bezier);
	}
	*/

	// scrivo il path calcolato
	if(n_strokes > 0  && strokes) {
	    g_sprintf(newpathname, "%s_%cR%.1f", pathname, (sw_abscissa_ascendent)? 'a' : 'b', points_per_pixel);
	    teseo_strokes_to_open_path(g_image, n_strokes, strokes, newpathname );
	} else {
	    // g_message("Strano: Il campionamento ha dato un risultato vuoto! sig ?!?");
	}

	g_free(strokes);

    } else {
	g_message("No selected path!");
    }

    g_free(punti_bezier);
}
