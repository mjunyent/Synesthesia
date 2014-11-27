//
//  ShotDetector.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "ShotDetector.h"

#include <Eigen/SVD>

ShotDetector::ShotDetector(HistogramHSV* histograms) : histograms(histograms) {
    
}

void ShotDetector::process() {
    for(currentFrame=0;
        currentFrame<histograms->histograms.size();
        currentFrame++) {
        findFlashes();
        findShots();
    }
    boundaries.push_back(histograms->histograms.size()+1);
}


void ShotDetector::findFlashes() {
    rgb current_mean = histograms->rgbMeans[currentFrame];
    bool lookHigh = true;
    int nofound = 0;
    double prediff = -1;
    rgb prergb;
    for(int i=currentFrame+1;
        nofound <= FLASH_MAX_FRAMES_UP && (i-currentFrame)<= FLASH_MAX_LENGTH && i < histograms->histograms.size();
        i++) {
        rgb i_mean = histograms->rgbMeans[i];
        double diff = fabs(current_mean.r-i_mean.r) + fabs(current_mean.g-i_mean.g) + fabs(current_mean.b-i_mean.b);
        
        if(lookHigh) {
            if(diff > FLASH_THRESHOLD_MAX) { prediff = diff; prergb = i_mean; lookHigh = false; }
            else if(diff < FLASH_THRESHOLD_MIN) break;
            else nofound++;
        } else {
            if(diff < FLASH_THRESHOLD_MIN) {
                for(int j=currentFrame+1; j<i; j++) flashes.insert(j);
                break;
            }
        }
    }
}


void ShotDetector::findShots() {
    if(flashes.find(currentFrame) != flashes.end() || currentFrame-WINDOW < -1) return;

    Eigen::MatrixXd m(histograms->numBins, WINDOW);

    int i=0;
    int j=0;
    while(i<WINDOW) {
        if(currentFrame-j < 0) return;
        if(flashes.find(currentFrame-j) != flashes.end()) {
            j++;
        } else {
            Eigen::VectorXd v(histograms->numBins);
            for(int k=0; k<histograms->numBins; k++) v[k] = histograms->histograms[currentFrame-j][k];

            m.col(i) = v;
            i++;
            j++;
        }
    }
    
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(m, Eigen::ComputeThinV | Eigen::ComputeThinU);
    Eigen::VectorXd s = svd.singularValues();
    s /= s[0];
    
    int next_rank = 0;
    for(int j=0; j<s.rows(); j++) if(s[j]>THRESHOLD) next_rank++;
    
    if(rank > previous_rank && previous_rank == 1) {
        if(boundaries.size()==0 || abs(currentFrame-boundaries[boundaries.size()-1]) > WINDOW) possibility = currentFrame-1;
    }
    
    if(rank > previous_rank) start_transition = true;
    
    if(possibility != 0 && start_transition && rank > next_rank) {
        boundaries.push_back(possibility);
        //        cerr << possibility << endl;
        start_transition = false;
        possibility = 0;
    }
    
    previous_rank = rank;
    rank = next_rank;
    //    std::cerr << currentFrame << "\t" << rank << std::endl;
}












