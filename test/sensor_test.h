/*!
 * @file   sensor_test.h
 * @brief  test program for force sensor
 * @author takahashi
 * @sa
 * @date   2016/09/09 New
 * @version    1.0
 */
#define AXIS_NUM          (6)
#define SAMPLING_RATE_HZ  (10) /* in Hz */
#define PRINT_PERIOD      (10)

#define SENSOR_DEVICE_CLASS_NAME() sensor_processor_wacoh_udynpick()

typedef struct _sensor_data {
    int tick;
    unsigned short data[AXIS_NUM];
} SENSOR_DATA;

class sensor_processor {
    public:
        void process(void);
    protected:
        virtual int init(void){};
        virtual int setup(void){};
        virtual int finalize(void){};
        virtual int get_data(_sensor_data *s){};
        virtual int show_data(_sensor_data *s){};
 };

class sensor_processor_wacoh_udynpick : public sensor_processor {
    private:
        int fd;
    protected:
        int init(void);
        int setup(void);
        int finalize(void);
        int get_data(_sensor_data *s);
        int show_data(_sensor_data *s);
};
