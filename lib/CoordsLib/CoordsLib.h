// https://github.com/juanrmn/Arduino-Telescope-Control
#ifndef CoordsLib_h
#define CoordsLib_h

	#include <math.h>

	/**
	 * \brief Library for coordinates transformations. Calculates the equivalent coordinates between both coordinate systems equatorial and horizontal.
	 *
	 * It's based on Toshimi Taki's matrix method for coordinates transformation: http://www.geocities.jp/toshimi_taki/matrix/matrix.htm
	 * Contains the necessary methods for setting the initial time, the reference objects, the transformation matrix, and to 
	 * calculate the equivalent vectors between both coordinate systems.
	 */
	class CoordsLib{
		private:

			/**
			 * Constant of multiplication for the solar and sidereal time relation.
			 */
			double _k;

			/**
			 * Initial timestamp for the observations.
			 */			
			double _t0;

			/**
			 * Indicators for definition of the three reference objects.
			 */
			bool _isSetR1, _isSetR2, _isSetR3;

			/**
			 * Auxiliary matrices.
			 */
			double _lmn1[3], _LMN1[3], _lmn2[3], _LMN2[3], _lmn3[3], _LMN3[3];

			/**
			 * Transformation matrix. Transform vectors from equatorial to horizontal system.
			 */
			double _T[3][3];

			/**
			 * Inverse transformation matrix. Transform vectors from horizontal to equatorial system.
			 */
			double _iT[3][3];

			/**
			 * If the three reference objects have been defined, it calculates the transformation matrix from them.
			 */
			void _setT();

			/**
			 * Obtains a vector in polar notation from the equatorial coordinates and the observation time.
			 *
			 * \param ar Right ascension.
			 * \param dec Declination.
			 * \param t Timestamp of the observation.
			 * \param *EVC Pointer to array: Returns the three dimensional vector in polar notation.
			 */
			void _setEVC(double ar, double dec, double t, double* EVC);

			/**
			 * Obtains a vector in polar notation from the horizontal coordinates and observation time.
			 *
			 * \param ac Azimuth.
			 * \param alt Altitude.
			 * \param t Timestamp of the observation.
			 * \param *HVC Pointer to array: Returns the three dimensional vector in polar notation.
			 */			
			void _setHVC(double ac, double alt, double* HVC);

			/**
			 * Calculates the 3x3 inverse matrix.
			 * 
			 * \param m[3][3] Input matrix.
			 * \param res[3][3] Pointer to array: Returns the inverse matrix.
			 */
			void _inv(double m[3][3], double res[3][3]);

			/**
			 * Calculates the product of 3x3 matrices.
			 * 
			 * \param m1[3][3] Input matrix 1.
			 * \param m2[3][3] Input matrix 2.
			 * \param res[3][3] Pointer to array: Returns the result matrix.
			 */
			void _m_prod(double m1[3][3], double m2[3][3], double res[3][3]);

		public:

			/**
			 * Class constructor.
			 */
			CoordsLib();

			/**
			 * Sets the initial time.
			 *
			 * This parameter is used in order to consider time passing on horizontal coordinates system.
			 *
			 * \param t0 Unix Timestamp of the initial observation time.
			 */
			void setTime(double t0);

			/**
			 * Sets the first reference object from the coordinates in both coordinates systems for 
			 * that object.
			 *
			 * \param ar Right Ascension (equatorial coordinates).
			 * \param dec Declination (equatorial coordinates).
			 * \param t Unix Timestamp of the Observation.
			 * \param ac Azimuth (horizontal coordinates).
			 * \param alt Altitude (horizontal coordinates).
			 */
			void setRef_1(double ar, double dec, double t, double ac, double alt);

			/**
			 * Sets the second reference object from the coordinates in both coordinates systems for 
			 * that object.
			 *
			 * \param ar Right Ascension (equatorial coordinates).
			 * \param dec Declination (equatorial coordinates).
			 * \param t Unix Timestamp of the Observation.
			 * \param ac Azimuth (horizontal coordinates).
			 * \param alt Altitude (horizontal coordinates).
			 */
			void setRef_2(double ar, double dec, double t, double ac, double alt);

			/**
			 * Sets the third reference object from the coordinates in both coordinates systems for 
			 * that object.
			 *
			 * \param ar Right Ascension (equatorial coordinates).
			 * \param dec Declination (equatorial coordinates).
			 * \param t Unix Timestamp of the Observation.
			 * \param ac Azimuth (horizontal coordinates).
			 * \param alt Altitude (horizontal coordinates).
			 */
			void setRef_3(double ar, double dec, double t, double ac, double alt);

			/**
			 * Indicates if the three reference objects has been calculated.
			 * 
			 * \return Boolean.
			 */
			bool isConfigured();

			/**
			 * Third reference object calculated from the two others ones.
			 * 
			 * Calculates the cross product of the two first reference objects in both coordinates systems, in order 
			 * to obtain the third one.
			 * These two first objects must have 90º from each other, approximately (from 60º to 120º is enough to obtain
			 * goods results).
			 */
			void autoRef_3();

			/**
			 * Horizontal coordinates calculated from the equatorial ones and time.
			 *
			 * \param ar Right Ascension (equatorial coordinates).
			 * \param dec Declination (equatorial coordinates)
			 * \param t Unix Timestamp of the Observation.
			 * \param *ac Pointer to double: Returns the azimuth (horizontal coordiantes).
			 * \param *alt Pointer to double: Returns the altitude (horizontal coordinates).
			 */
			void getHCoords(double ar, double dec, double t, double *ac, double *alt);

			/**
			 * Equatorial coordinates calculated from the horizontal ones and time.
			 *
			 * \param ac Azimuth (horizontal coordinates).
			 * \param alt Altitude (horizontal coordinates).
			 * \param t Unix Timestamp of the Observation.
			 * \param *ar Pointer to double: Returns the right ascension (equatorial coordinates).
			 * \param *dec Pointer to double: Returns the declination (equatorial coordinates).
			 */
			void getECoords(double ac, double alt, double t, double *ar, double *dec);
	};

#endif
