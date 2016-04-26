#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <openssl/md5.h>
#include <openssl/sha.h>

using namespace std;


// Debug mode
const bool DEBUG = false;

// Precision of floating point operations and representations
const double MZ_PRECISION = 6;
const long long MZ_PRECISION_FACTOR = static_cast<long long>(pow(10, MZ_PRECISION));

const double INTENSITY_PRECISION = 0;
const long long INTENSITY_PRECISION_FACTOR = static_cast<long long>(pow(10, INTENSITY_PRECISION));

const double EPS = 1.0e-6;

// Value to scale relative spectra
const double RELATIVE_INTENSITY_SCALE = 100.0;
const float RELATIVE_INTENSITY_SCALE_FLOAT = 100.0;

// Separator for building spectrum strings
const char ION_SEPARATOR = ' ';

// Full spectrum hash properties
const char ION_PAIR_SEPARATOR = ':';
const int MAX_HASH_CHARATERS_ENCODED_SPECTRUM = 20;
const double EPS_CORRECTION = 1.0e-7;



string sha256(const string s) {
    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s.c_str(), s.size());
    SHA256_Final(digest, &sha256);

    stringstream ss;

    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)digest[i];
    }

    return ss.str();
}

string md5(const string s) {
    unsigned char digest[MD5_DIGEST_LENGTH];

    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, s.c_str(), s.size());
    MD5_Final(digest, &md5);

    stringstream ss;

    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)digest[i];
    }

    return ss.str();
}


// http://stackoverflow.com/a/236803
vector<string> split(const string &s, char delimeter) {
    vector<string> elements;
    stringstream ss(s);
    string element;

    while(getline(ss, element, delimeter)) {
        elements.push_back(element);
    }

    return elements;
}


bool ionPairMzComparatorDouble(const pair<double, double> a, const pair<double, double> b) {
    if(abs(a.first - b.first) < EPS) {
        return a.second > b.second;
    } else {
        return a.first < b.first;
    }
}

bool ionPairMzComparatorFloat(const pair<float, float> a, const pair<float, float> b) {
    if(abs(a.first - b.first) < EPS) {
        return a.second > b.second;
    } else {
        return a.first < b.first;
    }
}

string buildSpectrumStringDouble(vector<pair<double, double> > &spectrum, char spectrum_type) {
    sort(spectrum.begin(), spectrum.end(), ionPairMzComparatorDouble);

    int i = 0;
    stringstream ss;

    for(vector<pair<double, double> >::iterator it = spectrum.begin(); it != spectrum.end(); ++it) {
        double mz = (*it).first;
        cerr << mz << endl;

        // Determine the m/z multiplier to scale to six decimal places of precision
        int power = 0;
        int multiplier = pow(10, power);
        double correction = pow(10, -7 + power);

        while((mz + correction) / multiplier > 1) {
            power++;
            multiplier = pow(10, power);
            correction = pow(10, -7 + power);
            cerr << "\t- " << power << " " << multiplier << " " << correction << " " << (mz + correction) / multiplier << " " << (mz + correction) * pow(10, 6 - power) << " " << static_cast<long long>((mz + correction) * pow(10, 6 - power)) << endl;
        }

        cerr << setprecision(10) << fixed << mz << " " << mz + correction << " " << (mz) * pow(10, 6 - power) << " " << (mz + correction) * pow(10, 6 - power) << endl;

        ss << static_cast<long long>((mz + correction) * pow(10, 6 - power))
           << ION_PAIR_SEPARATOR
           << static_cast<long long>(((*it).second + EPS_CORRECTION) * INTENSITY_PRECISION_FACTOR);

        if(++i < spectrum.size()) {
            ss << ION_SEPARATOR;
        }
    }

    if(DEBUG) {
        cerr << "Encoded Spectrum: '" << ss.str() << "'" << endl;
    }

    return ss.str();
}


string buildSpectrumStringFloat(vector<pair<float, float> > &spectrum, char spectrum_type) {
    sort(spectrum.begin(), spectrum.end(), ionPairMzComparatorFloat);

    int i = 0;
    stringstream ss;

    for(vector<pair<float, float> >::iterator it = spectrum.begin(); it != spectrum.end(); ++it) {
        float mz = (*it).first;
        cerr << mz << endl;

        // Determine the m/z multiplier to scale to six decimal places of precision
        int power = 0;
        int multiplier = pow(10, power);
        float correction = pow(10, -7 + power);

        while((mz + correction) / multiplier > 1) {
            power++;
            multiplier = pow(10, power);
            correction = pow(10, -7 + power);
            cerr << "\t- " << power << " " << multiplier << " " << correction << " " << (mz + correction) / multiplier << " " << (mz + correction) * pow(10, 6 - power) << " " << static_cast<long long>((mz + correction) * pow(10, 6 - power)) << endl;
        }

        cerr << setprecision(10) << fixed << mz << " " << mz + correction << " " << (mz) * pow(10, 6 - power) << " " << (mz + correction) * pow(10, 6 - power) << endl;

        ss << static_cast<long long>((mz + correction) * pow(10, 6 - power))
           << ION_PAIR_SEPARATOR
           << static_cast<long long>(((*it).second + EPS_CORRECTION) * INTENSITY_PRECISION_FACTOR);

        if(++i < spectrum.size()) {
            ss << ION_SEPARATOR;
        }
    }

    if(DEBUG) {
        cerr << "Encoded Spectrum: '" << ss.str() << "'" << endl;
    }

    return ss.str();
}


string encodeSpectrum(string s) {
    return sha256(s).substr(0, MAX_HASH_CHARATERS_ENCODED_SPECTRUM);
}




string splashIt_double(vector<pair<double, double> > &spectrum, char spectrum_type) {
    stringstream ss;
    string s = buildSpectrumStringDouble(spectrum, spectrum_type);
    cout << s << endl;
    ss << encodeSpectrum(s);// << "\t" << md5(s);
    return ss.str();
}

string splashIt_float(vector<pair<float, float> > &spectrum, char spectrum_type) {
    stringstream ss;
    string s = buildSpectrumStringFloat(spectrum, spectrum_type);
    cout << s << endl;
    ss << encodeSpectrum(s);// << "\t" << md5(s);
    return ss.str();
}

string splashIt(string spectrum_string, char spectrum_type) {
    // Convert spectrum to a vector of ion pairs and find the max intensity
    vector<string> ion_strings = split(spectrum_string, ' ');
    vector<pair<double, double> > spectrum_double;
    vector<pair<double, double> > spectrum_float;

    double maxIntensity = 0;
    float maxIntensity_float = 0;

    for(vector<string>::iterator it = ion_strings.begin(); it != ion_strings.end(); ++it) {
        int delim_pos = (*it).find(':');

        double mz = stod((*it).substr(0, delim_pos));
        double intensity = stod((*it).substr(delim_pos + 1));

        float mz_float = stof((*it).substr(0, delim_pos));
        float intensity_float = stof((*it).substr(delim_pos + 1));

        if(intensity > maxIntensity)
            maxIntensity = intensity;

        if(intensity_float > maxIntensity_float)
            maxIntensity_float = intensity_float;

        // Store ion as a pair object, with 'first' corresponding to m/z
        // and 'second' to intensity
        spectrum_double.push_back(make_pair(mz, intensity));
        spectrum_float.push_back(make_pair(mz_float, intensity_float));
    }

    // Normalize spectrum
    for(vector<pair<double, double> >::iterator it = spectrum_double.begin(); it != spectrum_double.end(); ++it) {
        (*it).second = (*it).second / maxIntensity * RELATIVE_INTENSITY_SCALE;
    }
    for(vector<pair<double, double> >::iterator it = spectrum_float.begin(); it != spectrum_float.end(); ++it) {
        (*it).second = (*it).second / maxIntensity_float * RELATIVE_INTENSITY_SCALE_FLOAT;
    }
    
    // Return the calculated splash id
    stringstream ss;
    ss << splashIt_double(spectrum_double, '1') << "\t" << splashIt_double(spectrum_float, '1');
    return ss.str();
}