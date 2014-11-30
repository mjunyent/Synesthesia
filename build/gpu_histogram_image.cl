#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

/***************************************************************************************************************/

//
// sum partial histogram results into final histogram bins
//
// num_groups is the number of work-groups used to compute partial histograms.
// partial_histogram is an array of num_groups * (256 * 3 * 32-bits/entry) entries
// we store 256 Red bins, followed by 256 Green bins and then the 256 Blue bins.
//
// final summed results are returned in histogram.
//
kernel
void histogram_sum_partial_results_unorm8(global uint *partial_histogram, int num_groups, global uint *histogram)
{
    int     tid = (int)get_global_id(0);
    int     group_indx;
    int     n = num_groups;
    local uint  tmp_histogram[256 * 3];

    tmp_histogram[tid] = partial_histogram[tid];
    
    group_indx = 256*3;
    while (--n > 0)
    {
        tmp_histogram[tid] += partial_histogram[group_indx + tid];
        group_indx += 256*3;
    }
    
    histogram[tid] = tmp_histogram[tid];
}

//
// this kernel takes a RGBA 8-bit / channel input image and produces a partial histogram.
// the kernel is executed over multiple work-groups.  for each work-group a partial histogram is generated
// partial_histogram is an array of num_groups * (256 * 3 * 32-bits/entry) entries
// we store 256 Red bins, followed by 256 Green bins and then the 256 Blue bins.
//
kernel
void histogram_image_rgba_unorm8(image2d_t img, global uint *histogram)
{
    int     local_size = (int)get_local_size(0) * (int)get_local_size(1);
    int     image_width = get_image_width(img);
    int     image_height = get_image_height(img);
    int     group_indx = (get_group_id(1)*get_num_groups(0) + get_group_id(0))*24; // mad24(get_group_id(1), get_num_groups(0), get_group_id(0)) * 256 * 3;
    int     x = get_global_id(0);
    int     y = get_global_id(1);
    
    local uint  tmp_histogram[24];
    
    int     tid = get_local_id(1)*get_local_size(0)+get_local_id(0); // mad24(get_local_id(1), get_local_size(0), get_local_id(0));
    int     j = 24;
    int     indx = 0;
    
    // clear the local buffer that will generate the partial histogram
    do
    {
        if (tid < j)
            tmp_histogram[indx+tid] = 0;

        j -= local_size;
        indx += local_size;
    } while (j > 0);
    
    barrier(CLK_LOCAL_MEM_FENCE);
    
    if ((x < image_width) && (y < image_height))
    {
        int write = 1;
        float4 clr = read_imagef(img, CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST, (float2)(x, y));

        float big = max(clr.x, clr.y);
        big = max(big, clr.z);
        
        float small = min(clr.x, clr.y);
        small = min(small, clr.z);

        float h;
        float s;
        float v = big;
        
        float delta = big-small;
        
        if(big > 0.0f) s = delta/big;
        else {
            s = 0.0f;
            write = 0;
        }
        
        if(clr.x >= big) {
            h = (clr.y - clr.z)/delta;
        } else {
            if(clr.y >= big) {
                h = 2.0f + (clr.z - clr.x)/delta;
            } else {
                h = 4.0f + (clr.x - clr.y)/delta;
            }
        }
        
        h *= 60.0f;
        
        if(h < 0.0f) h += 360.0f;

        uint ph = (uint)min(15.0f,(h/360.0f*16.0f));
        uint ps = (uint)min(3.0f,(s*4.0f));
        uint pv = (uint)min(3.0f,(v*4.0f));

        if(write == 1) {
            atom_inc(&tmp_histogram[ph]);
        }
        atom_inc(&tmp_histogram[16+ps]);
        atom_inc(&tmp_histogram[20+pv]);

        /*
        uchar   indx_x, indx_y, indx_z;
        indx_x = convert_uchar_sat(clr.x * 255.0f);
        indx_y = convert_uchar_sat(clr.y * 255.0f);
        indx_z = convert_uchar_sat(clr.z * 255.0f);
        atom_inc(&tmp_histogram[indx_x]);
        atom_inc(&tmp_histogram[256+(uint)indx_y]);
        atom_inc(&tmp_histogram[512+(uint)indx_z]);*/
    }
    
    barrier(CLK_LOCAL_MEM_FENCE);

    // copy the partial histogram to appropriate location in histogram given by group_indx
    if (local_size >= (24))
    {
        if (tid < (24))
            histogram[group_indx + tid] = tmp_histogram[tid];
    }
    else
    {
        j = 24;
        indx = 0;
        do 
        {
            if (tid < j)
                histogram[group_indx + indx + tid] = tmp_histogram[indx + tid];
                
            j -= local_size;
            indx += local_size;
        } while (j > 0);
    }
}




