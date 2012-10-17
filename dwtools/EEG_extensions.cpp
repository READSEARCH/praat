/* EEG_extensions.cpp
 *
 * Copyright (C) 2012 David Weenink
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "ICA.h"
#include "EEG.h"
#include "NUM2.h"
#include "Sound_and_PCA.h"
#include "Sound_extensions.h"

static EEG EEG_copyWithoutSound (EEG me) {
	try {
 		autoEEG thee = EEG_create (my xmin, my xmax);
		thy d_numberOfChannels = my d_numberOfChannels;
		thy d_textgrid = (TextGrid) Data_copy (my d_textgrid);
		autostringvector channelNames (1, my d_numberOfChannels);
		for (long i = 1; i <= my d_numberOfChannels; i++) {
			channelNames[i] = Melder_wcsdup (my d_channelNames[i]);
		}
		thy d_channelNames = channelNames.transfer();
		return thee.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": not copied.");
	}
}

static long *EEG_channelNames_to_channelNumbers (EEG me, wchar_t **channelNames, long numberOfChannelNames) {
	try {
		autoNUMvector<long> channelNumbers (1, numberOfChannelNames);
		for (long i = 1; i <= numberOfChannelNames; i++) {
			for (long j = 1; j <= my d_numberOfChannels; j++) {
				if (Melder_wcsequ (channelNames[i], my d_channelNames[j])) {
					channelNumbers[i] = j;
				}
			}
			if (channelNumbers[i] == 0) {
				Melder_throw ("Channel name \"", channelNames[i], "\" not found.");
			}
		}
		return channelNumbers.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": channelNames not found.");
	}
}

static void EEG_setChannelNames_selected (EEG me, const wchar_t *precursor, long *channelNumbers, long numberOfChannels) {
	autoMelderString name;
	const wchar_t *zero = L"0";
	for (long i = 1; i <= numberOfChannels; i++) {
		MelderString_append (&name, precursor);
		if (my d_numberOfChannels > 100) {
			if (i < 10) {
				MelderString_append (&name, zero);
			}
			if (i < 100) {
				MelderString_append (&name, zero);
			}
		} else if (i < 10) {
			MelderString_append (&name, zero);
		}
		MelderString_append (&name, Melder_integer (i));
		my f_setChannelName (channelNumbers[i], name.string);
		MelderString_empty (&name);
	}
}

CrossCorrelationTable EEG_to_CrossCorrelationTable (EEG me, double startTime, double endTime, double lagTime, const wchar_t *channelRanges)
{
	try {
		// autowindow
		if (startTime == endTime) {
			startTime = my xmin; endTime = my xmax;
		}
		// don't allow times outside domain
		if (startTime < my xmin) {
			startTime = my xmin;
		}
		if (endTime > my xmax) {
			endTime = my xmax;
		}
		autoEEG thee = my f_extractPart (startTime, endTime, true);
		long numberOfChannels;
		autoNUMvector <long> channels (NUMstring_getElementsOfRanges (channelRanges, thy d_numberOfChannels, & numberOfChannels, NULL, L"channel", true), 1);
		autoSound soundPart = Sound_copyChannelRanges (thy d_sound, channelRanges);
		autoCrossCorrelationTable him = Sound_to_CrossCorrelationTable (soundPart.peek(), startTime, endTime, lagTime);
		// assign channel names
		for (long i = 1; i <= numberOfChannels; i++) {
			long ichannel = channels[i];
			wchar_t *label = my d_channelNames[ichannel];
			TableOfReal_setRowLabel (him.peek(), i, label);
			TableOfReal_setColumnLabel (him.peek(), i, label);
		}
		return him.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": no CrossCorrelationTable calculated.");
	}
}

Covariance EEG_to_Covariance (EEG me, double startTime, double endTime, const wchar_t *channelRanges)
{
	try {
		double lagTime = 0.0;
		autoCrossCorrelationTable thee = EEG_to_CrossCorrelationTable (me, startTime, endTime, lagTime, channelRanges);
        autoCovariance him = Thing_new (Covariance);
        thy structCrossCorrelationTable :: v_copy (him.peek());
		return him.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": no Covariance calculated.");
	}
}

CrossCorrelationTables EEG_to_CrossCorrelationTables (EEG me, double startTime, double endTime, double lagTime, long ncovars, const wchar_t *channelRanges) {
	try {
		// autowindow
		if (startTime == endTime) {
			startTime = my xmin; endTime = my xmax;
		}
		// don't allow times outside domain
		if (startTime < my xmin) {
			startTime = my xmin;
		}
		if (endTime > my xmax) {
			endTime = my xmax;
		}
		autoEEG thee = my f_extractPart (startTime, endTime, true);
		long numberOfChannels;
		autoNUMvector <long> channels (NUMstring_getElementsOfRanges (channelRanges, thy d_numberOfChannels, & numberOfChannels, NULL, L"channel", true), 1);
		autoSound soundPart = Sound_copyChannelRanges (thy d_sound, channelRanges);
		autoCrossCorrelationTables him = Sound_to_CrossCorrelationTables (soundPart.peek(), startTime, endTime, lagTime, ncovars);
		return him.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": no CrossCorrelationTables calculated.");
	}
}

PCA EEG_to_PCA (EEG me, double startTime, double endTime, const wchar_t *channelRanges, int fromCorrelation) {
	try {
		autoCovariance cov = EEG_to_Covariance (me, startTime, endTime, channelRanges);
		if (fromCorrelation) {
			autoCorrelation cor = SSCP_to_Correlation (cov.peek());
			autoPCA him = SSCP_to_PCA (cor.peek());
			return him.transfer();
		} else {
			autoPCA him = SSCP_to_PCA (cov.peek());
			return him.transfer();
		}
	} catch (MelderError) {
		Melder_throw (me, ": no PCA calculated.");
	}
}

EEG EEG_and_PCA_to_EEG_whiten (EEG me, PCA thee, long numberOfComponents) {
	try {
		if (numberOfComponents <= 0 || numberOfComponents > thy numberOfEigenvalues) {
			numberOfComponents = thy numberOfEigenvalues;
		}
		numberOfComponents = numberOfComponents > my d_numberOfChannels ? my d_numberOfChannels : numberOfComponents;

		autoNUMvector<long> channelNumbers (EEG_channelNames_to_channelNumbers (me, thy labels, thy dimension), 1);

		autoEEG him = (EEG) Data_copy (me);
		autoSound white = Sound_and_PCA_whitenSelectedChannels (my d_sound, thee, numberOfComponents, channelNumbers.peek(), thy dimension);
		for (long i = 1; i <= thy dimension; i++) {
			long ichannel = channelNumbers[i];
			NUMvector_copyElements<double> (white -> z[i], his d_sound -> z[ichannel], 1, his d_sound -> nx);
		}
		EEG_setChannelNames_selected (him.peek(), L"wh", channelNumbers.peek(), thy dimension);
		return him.transfer();
	} catch(MelderError) {
		Melder_throw (me, ": not whitened with ", thee);
	}
}

EEG EEG_and_PCA_to_EEG_principalComponents (EEG me, PCA thee, long numberOfComponents) {
	try {
		if (numberOfComponents <= 0 || numberOfComponents > thy numberOfEigenvalues) {
			numberOfComponents = thy numberOfEigenvalues;
		}
		numberOfComponents = numberOfComponents > my d_numberOfChannels ? my d_numberOfChannels : numberOfComponents;

		autoNUMvector<long> channelNumbers (EEG_channelNames_to_channelNumbers (me, thy labels, thy dimension), 1);
		autoEEG him = (EEG) Data_copy (me);
		autoSound pc = Sound_and_PCA_to_Sound_pc_selectedChannels (my d_sound, thee, numberOfComponents, channelNumbers.peek(), thy dimension);
		for (long i = 1; i <= thy dimension; i++) {
			long ichannel = channelNumbers[i];
			NUMvector_copyElements<double> (pc -> z[i], his d_sound -> z[ichannel], 1, his d_sound -> nx);
		}
		EEG_setChannelNames_selected (him.peek(), L"pc", channelNumbers.peek(), thy dimension);
		return him.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": not projected.");
	}
}

EEG EEG_to_EEG_ICA (EEG me, double startTime, double endTime, long ncovars, double lagTime, const wchar_t *channelRanges, long maxNumberOfIterations, double tol, int whitenFirst, int fromCorrelation, long numberOfComponents,  int method) {
	try {
		// autowindow
		if (startTime == endTime) {
			startTime = my xmin; endTime = my xmax;
		}
		// don't allow times outside domain
		if (startTime < my xmin) {
			startTime = my xmin;
		}
		if (endTime > my xmax) {
			endTime = my xmax;
		}
		autoEEG thee = my f_extractPart (startTime, endTime, true);
		if (whitenFirst) {
			autoPCA pca = EEG_to_PCA (thee.peek(), thy xmin, thy xmax, channelRanges, false);
			autoEEG white = EEG_and_PCA_to_EEG_whiten (thee.peek(), pca.peek(), numberOfComponents);
			thee.reset (white.transfer());
		}


	} catch (MelderError) {
		Melder_throw (me, ": no independent components determined.");
	}
}

/* End of file EEG_extensions.cpp */
